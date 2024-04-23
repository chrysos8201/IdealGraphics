#include "IdealRenderer.h"
#include "GraphicsEngine/VertexInfo.h"
#include <DirectXColors.h>
#include "Misc/Utils/PIX.h"

#include "GraphicsEngine/Resource/Mesh.h"
#include "Misc/AssimpLoader.h"

// Test
#include "Misc/Assimp/AssimpConverter.h"
#include "GraphicsEngine/Resource/Model.h"
//#include "ThirdParty/DirectXTex/DirectXTex.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/D3D12/D3D12ResourceManager.h"

IdealRenderer::IdealRenderer(HWND hwnd, uint32 width, uint32 height)
	: m_hwnd(hwnd),
	m_width(width),
	m_height(height),
	m_viewport(hwnd, width, height)
{
	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

IdealRenderer::~IdealRenderer()
{
	int a = 3;
	//uint32 ref_count = m_device->Release();

	// Release Resource Manager
	m_resourceManager = nullptr;
}

void IdealRenderer::Init()
{
	uint32 dxgiFactoryFlags = 0;

#ifdef _DEBUG
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}

#endif

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ComPtr<ID3D12Debug1> debugController1;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
			{
				debugController1->SetEnableGPUBasedValidation(true);
			}
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// 2024.04.14 // 임시용 wvp만들기
	{
		Vector3 eyePos(0.f, 0.f, 5.f);
		Vector3 targetPos = Vector3::Zero;
		Vector3 upward(0.f, 1.f, 0.f);

		auto fov = DirectX::XMConvertToRadians(37.5f);
		auto aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

		Matrix world = Matrix::Identity;
		Matrix view = Matrix::CreateLookAt(eyePos, targetPos, upward);
		Matrix proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 0.3f, 1000.f);
		m_view = view;
		m_proj = proj;
		m_viewProj = view * proj;
	}

	// Ideal Viewport Init
	m_viewport.Init();

	ComPtr<IDXGIFactory4> factory;
	Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf())));

	D3D_FEATURE_LEVEL	featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	
	DWORD	FeatureLevelNum = _countof(featureLevels);
	ComPtr<IDXGIAdapter1> adapter = nullptr;
	DXGI_ADAPTER_DESC1 adapterDesc = {};
	for (DWORD featerLevelIndex = 0; featerLevelIndex < FeatureLevelNum; featerLevelIndex++)
	{
		UINT adapterIndex = 0;
		while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
		{
			adapter->GetDesc1(&adapterDesc);

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevels[featerLevelIndex], IID_PPV_ARGS(m_device.GetAddressOf()))))
			{
				DXGI_ADAPTER_DESC1 desc = {};
				adapter->GetDesc1(&desc);

				goto finishAdapter;
				break;
			}
			adapter = nullptr;
			adapterIndex++;
		}
	}
finishAdapter:
	//ComPtr<IDXGIAdapter1> hardwareAdapter;
	//Check(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.GetAddressOf())));


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	Check(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf())));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_BUFFER_COUNT;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	Check(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		swapChain.GetAddressOf()
	));

	Check(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

	Check(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// rtv descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Check(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	// descriptor heap 에서 rtv Descriptor의 크기
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			Check(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);

			//Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		}
	}

	// 2024.04.22
	//------------Create Command List-------------//
	CreateCommandList();

	//------------Create Fence---------------//
	// create Fence
	//CreateFence();
	CreateGraphicsFence();

	// 2024.04.14 : dsv를 만들겠다. 먼저 descriptor heap을 만든다.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Check(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

	{
		D3D12_CLEAR_VALUE depthClearValue = {};
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthClearValue.DepthStencil.Depth = 1.0f;
		depthClearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			m_width,
			m_height,
			1,
			0,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		m_device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthClearValue,
			IID_PPV_ARGS(m_depthStencil.GetAddressOf())
		);

		// create dsv
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// 2024.04.21 Ideal::Descriptor Heap
	m_idealSrvHeap.Create(shared_from_this(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_srvHeapNum);



	////////////////////////////////////// Load ASSET
	// 2024.04.18 Convert to my format
	std::shared_ptr<AssimpConverter> assimpConverter = std::make_shared<AssimpConverter>();
	/*
	assimpConverter->ReadAssetFile(L"statue_chronos/statue_join.fbx");
	assimpConverter->ExportModelData(L"statue_chronos/statue_chronos");
	assimpConverter->ExportMaterialData(L"statue_chronos/statue_chronos");
	assimpConverter.reset();
	*/
	//assimpConverter = std::make_shared<AssimpConverter>();
	//assimpConverter->ReadAssetFile(L"Tower/Tower.fbx");
	//assimpConverter->ExportModelData(L"Tower/Tower");
	//assimpConverter->ExportMaterialData(L"Tower/Tower");
	//assimpConverter.reset();
	//
	//assimpConverter = std::make_shared<AssimpConverter>();
	//assimpConverter->ReadAssetFile(L"House2/untitled.fbx");
	//assimpConverter->ExportModelData(L"House2/House2");
	//assimpConverter->ExportMaterialData(L"House2/House2");
	//assimpConverter.reset();

	//m_model = std::make_shared<Ideal::Model>();
	//m_model->ReadModel(L"porsche/porsche");	// mesh 밖에 없음.
	//m_model->ReadMaterial(L"porsche/porsche");


	//------------------Resource Manager---------------------//
	m_resourceManager = std::make_shared<Ideal::D3D12ResourceManager>();
	m_resourceManager->Init(m_device);


	// Test
	m_testVB = std::make_shared<Ideal::D3D12VertexBuffer>();
	m_resourceManager->CreateVertexBufferBox(m_testVB);
	//TestCreateVertexBuffer(m_testVB);
	// Test
	m_testIB = std::make_shared<Ideal::D3D12IndexBuffer>();
	m_resourceManager->CreateIndexBufferBox(m_testIB);

	LoadAssets();
	LoadBox();
}

void IdealRenderer::Tick()
{
	BoxTick();

	for (auto m : m_models)
	{
		m->Tick(m_frameIndex);
	}
}

void IdealRenderer::Render()
{
	BeginRender();

	//-------------Render Command-------------//
	{
		ID3D12DescriptorHeap* heaps[] = { m_resourceManager->GetSRVHeap().Get() };
		m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

		DrawBox();
		// Test Models Render
		for (auto m : m_models)
		{
			m->Render(shared_from_this(), m_commandList.Get(), m_frameIndex);
		}
	}/*
	ID3D12DescriptorHeap* heaps[] = { m_idealSrvHeap.GetDescriptorHeap().Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);*/

	//DrawBox();
	EndRender();

	//----------------Present-------------------//
	GraphicsPresent();
	//Present();

	return;
}

void IdealRenderer::Release()
{

}

void IdealRenderer::LoadAssets()
{
	CreateMeshObject(L"Tower/Tower");
	//CreateMeshObject(L"House2/House2");
	//LoadBox();
	//m_commandList->Close();
}

void IdealRenderer::CreateMeshObject(const std::wstring FileName)
{
	std::shared_ptr<Ideal::Model> model = std::make_shared<Ideal::Model>();
	model->ReadModel(FileName);
	model->ReadMaterial(FileName);
	model->Create(shared_from_this());
	m_models.push_back(model);
}

void IdealRenderer::BeginRender()
{
	//Check(m_commandAllocators[m_frameIndex]->Reset());
	//Check(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

	Check(m_commandAllocator->Reset());
	Check(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	// 2024.04.15 pipeline state를 m_currentPipelineState에 있는 것으로 세팅한다.

	m_commandList->RSSetViewports(1, &m_viewport.GetViewport());
	m_commandList->RSSetScissorRects(1, &m_viewport.GetScissorRect());

	CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferRenderTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	// 2024.04.14 dsv세팅
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::DimGray, 0, nullptr);
	// 2024.04.14 Clear DSV
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void IdealRenderer::EndRender()
{
	CD3DX12_RESOURCE_BARRIER backBufferPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &backBufferPresent);

	Check(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

std::shared_ptr<Ideal::D3D12ResourceManager> IdealRenderer::GetResourceManager()
{
	return m_resourceManager;
}

Microsoft::WRL::ComPtr<ID3D12Device> IdealRenderer::GetDevice()
{
	return m_device;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> IdealRenderer::GetCommandList()
{
	return m_commandList;
}

uint32 IdealRenderer::GetFrameIndex() const
{
	return m_frameIndex;
}

void IdealRenderer::LoadBox()
{
	{
		// CBV를 다시 만들었으니 Root Signature을 다시 만든다.
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		// 2024.04.13 : 루트 시그니쳐 뭔가 바꿔보겠다
		// 1개 있고, b0에 들어갈 것이고, data_static은 일반적으로 상수 버퍼 설정에 넣는 것이라고 한다(?), 그리고 이 데이터는 vertex 쉐이더에서 사용할 것이다
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);


		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	CreateBoxTexPipeline();
	CreateBoxTexture();

	// 2024.04.13 : ConstantBuffer 다시 만든다.
		// 프레임 개수 만큼 메모리를 할당 할 것이다.
	{
		const uint32 testConstantBufferSize = sizeof(SimpleBoxConstantBuffer);

		m_idealConstantBuffer.Create(m_device.Get(), testConstantBufferSize, FRAME_BUFFER_COUNT);

		m_simpleBoxConstantBufferDataBegin = (SimpleBoxConstantBuffer*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
	}
	return;

	{
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// 업로드 용으로 버퍼 하나를 만드는 것 같다.
		const VertexTest2 cubeVertices[] = {
			{ { -0.5f, 0.5f, -0.5f},	Vector2(0.0f, 1.0f) },    // Back Top Left
			{ {  0.5f, 0.5f, -0.5f},	Vector2(0.0f, 0.0f) },    // Back Top Right
			{ {  0.5f, 0.5f,  0.5f},	Vector2(1.0f, 0.0f) },    // Front Top Right
			{ { -0.5f, 0.5f,  0.5f},	Vector2(1.0f, 1.0f)	},    // Front Top Left

			{ { -0.5f, -0.5f,-0.5f},	Vector2(0.0f, 1.0f)	},    // Back Bottom Left
			{ {  0.5f, -0.5f,-0.5f},	Vector2(0.0f, 0.0f) },    // Back Bottom Right
			{ {  0.5f, -0.5f, 0.5f},	Vector2(1.0f, 0.0f) },    // Front Bottom Right
			{ { -0.5f, -0.5f, 0.5f},	Vector2(1.0f, 1.0f) },    // Front Bottom Left
		};
		const uint32 vertexBufferSize = sizeof(cubeVertices);

		// UploadBuffer를 만든다.
		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(m_device.Get(), vertexBufferSize);

		void* mappedUploadHeap = uploadBuffer.Map();
		memcpy(mappedUploadHeap, cubeVertices, sizeof(cubeVertices));
		uploadBuffer.UnMap();

		uint32 vertexTypeSize = sizeof(VertexTest2);
		uint32 vertexCount = _countof(cubeVertices);
		m_idealVertexBuffer.Create(m_device.Get(), m_commandList.Get(), vertexTypeSize, vertexCount, uploadBuffer);
		m_commandList->Close();
		//ExecuteCommandList();
		// 자 vertexBuffer를 GPU에 복사해주어야 하니까 Wait를 일단 걸어준다.
		// command List를 일단 닫아주고 실행시킨다.
		//{
		//	Check(m_commandList->Close());
		//	ID3D12CommandList* commandLists[] = { m_commandList.Get() };	// 하나밖에 없다
		//	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		//	// 동기화 오브젝트를 만들고 에셋 정보가 GPU에 업로드되기까지 기다린다.
		//	{
		//		Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		//		m_fenceValues[m_frameIndex]++;

		//		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//		if (m_fenceEvent == nullptr)
		//		{
		//			Check(HRESULT_FROM_WIN32(GetLastError()));
		//		}

		//		WaitForGPU();
		//	}
		//}
	}
	//m_commandAllocators[m_frameIndex]->Reset();
	//m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	// Index Buffer도 만들겠다. 여기서도 아마 wait을 걸 것 같기는 한데 해결하는 전략이 있어보이긴 하지만 일단은 그냥 wait걸어서 만들겠따.
	{
		uint32 indices[] = {
			0, 1, 3,
			1, 2, 3,

			3, 2, 7,
			6, 7, 2,

			2, 1, 6,
			5, 6, 1,

			1, 0, 5,
			4, 5, 0,

			0, 3, 4,
			7, 4, 3,

			7, 6, 4,
			5, 4, 6, };
		const uint32 indexBufferSize = sizeof(indices);

		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(m_device.Get(), indexBufferSize);


		// 업로드버퍼에 먼저 복사
		void* mappedUploadBuffer = uploadBuffer.Map();
		memcpy(mappedUploadBuffer, indices, indexBufferSize);
		uploadBuffer.UnMap();

		uint32 indexTypeSize = sizeof(uint32);
		uint32 indexCount = _countof(indices);

		m_idealIndexBuffer.Create(m_device.Get(), m_commandList.Get(), indexTypeSize, indexCount, uploadBuffer);
		//ExecuteCommandList();
		// 일단 커맨드 리스트를 닫는다
		//{
		//	Check(m_commandList->Close());
		//	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
		//	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
		//	// 여기서도 Wait를 걸면 될까?
		//	{
		//		Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		//		m_fenceValues[m_frameIndex]++;

		//		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//		if (m_fenceEvent == nullptr)
		//		{
		//			Check(HRESULT_FROM_WIN32(GetLastError()));
		//		}

		//		WaitForGPU();
		//	}
		//}
		m_commandList->Close();
		//ExecuteCommandList();


	}

	return;
}

void IdealRenderer::CreateBoxTexPipeline()
{
	// LinearWrap
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	// Root Signature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 descRange[1];
	descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameter[2];
	rootParameter[0].InitAsDescriptorTable(1, &descRange[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameter[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc(2, rootParameter, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	Check(D3DX12SerializeVersionedRootSignature(&rootSigDesc, featureData.HighestVersion, &signature, &error));
	Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_texRootSignature.GetAddressOf())));

	ComPtr<ID3DBlob> errorBlob;
	uint32 compileFlags = 0;

	D3D12_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	// Set VS
	ComPtr<ID3DBlob> vertexShader;
	Check(D3DCompileFromFile(
		L"Shaders/SimpleVertexShaderUV.hlsl",
		nullptr,
		nullptr,
		"VS",
		"vs_5_0",
		compileFlags, 0, &vertexShader, &errorBlob));

	// Set PS
	ComPtr<ID3DBlob> pixelShader;
	Check(D3DCompileFromFile(
		L"Shaders/SimpleVertexShaderUV.hlsl",
		nullptr,
		nullptr,
		"PS",
		"ps_5_0",
		compileFlags, 0, &pixelShader, &errorBlob));


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Set Input Layout
	psoDesc.InputLayout = { inputElements, 2 };

	psoDesc.pRootSignature = m_texRootSignature.Get();

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;


	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	Check(m_device->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(m_pipelineState.GetAddressOf())
	));
}

void IdealRenderer::CreateBoxTexture()
{
	m_texture = std::make_shared<Ideal::D3D12Texture>();
	std::wstring path = L"Resources/Test/test2.png";
	m_resourceManager->CreateTexture(m_texture, path);
	return;

	//----------------Load WIC Texture From File----------------//

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subResource;
	Check(DirectX::LoadWICTextureFromFile(
		m_device.Get(),
		L"Resources/Test/test2.png",
		m_tex.ReleaseAndGetAddressOf(),
		decodedData,
		subResource
	));

	//----------------Create Upload Buffer----------------//

	uint64 uploadBufferSize = GetRequiredIntermediateSize(m_tex.Get(), 0, 1);
	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	ComPtr<ID3D12Resource> uploadBuffer;
	Check(m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	//----------------Update Subresources----------------//

	UpdateSubresources(
		m_commandList.Get(), m_tex.Get(),
		uploadBuffer.Get(),
		0, 0, 1, &subResource
	);
	m_tex->SetName(L"Test");

	//----------------Resource Barrier----------------//

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_tex.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();
	//ExecuteCommandList();
	//----------------Create Shader Resource View----------------//

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_tex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	//m_device->CreateShaderResourceView(m_tex.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateShaderResourceView(m_tex.Get(), &srvDesc, m_texHandle.GetCpuHandle());
	//m_commandList->Close();
}

void IdealRenderer::DrawBox()
{
	m_commandList->SetPipelineState(m_pipelineState.Get());

	//---------------------IA--------------------//
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//auto vbView = m_idealVertexBuffer.GetView();
	auto vbView = m_testVB->GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbView);
	//auto ibView = m_idealIndexBuffer.GetView();
	auto ibView = m_testIB->GetView();
	m_commandList->IASetIndexBuffer(&ibView);

	//---------------------Root Signature--------------------//
	m_commandList->SetGraphicsRootSignature(m_texRootSignature.Get());

	//ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	//ID3D12DescriptorHeap* heaps[] = { m_idealSrvHeap.GetDescriptorHeap().Get() };
	ID3D12DescriptorHeap* heaps[] = { m_resourceManager->GetSRVHeap().Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	//m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	//m_commandList->SetGraphicsRootDescriptorTable(0, m_texHandle.GetGpuHandle());
	m_commandList->SetGraphicsRootConstantBufferView(1, m_idealConstantBuffer.GetGPUVirtualAddress(m_frameIndex));

	//---------------------Draw--------------------//
	//m_commandList->DrawIndexedInstanced(m_idealIndexBuffer.GetElementCount(), 1, 0, 0, 0);
	m_commandList->DrawIndexedInstanced(m_testIB->GetElementCount(), 1, 0, 0, 0);

}

void IdealRenderer::BoxTick()
{
	// 2024.04.14 constantBuffer 수정
	//m_constantBufferDataSimpleBox
	static float rot = 0.0f;
	m_simpleBoxConstantBufferDataBegin = (SimpleBoxConstantBuffer*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
	Matrix mat = Matrix::CreateRotationY(rot) * Matrix::CreateRotationX(-rot) * m_viewProj;
	Matrix tMat = mat.Transpose();
	rot += 0.01f;
	m_simpleBoxConstantBufferDataBegin->worldViewProjection = tMat;
}

void IdealRenderer::CreateCommandList()
{
	// 프레임 버퍼만큼 만들어준다.
	/*for (uint32 i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));

		WCHAR name[50];
		if (swprintf_s(name, L"Allocator[%d]", i) > 0)
		{
			m_commandAllocators[i]->SetName(name);
		}
	}

	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
	*/
	Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
	m_commandList->Close();
}

//void IdealRenderer::CreateFence()
//{
//	m_fenceValues[0] = 0;
//	m_fenceValues[1] = 0;
//
//	Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));
//	m_fenceValues[m_frameIndex]++;
//	m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
//	m_fence->SetName(L"My Graphics Fence");
//}
//
//void IdealRenderer::Present()
//{
//	Check(m_swapChain->Present(0, 0));
//
//	const uint64 currentFenceValue = m_fenceValues[m_frameIndex];
//	Check(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));
//
//	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
//
//	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
//	{
//		m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
//		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
//	}
//
//	// 다음 프레임을 위해 fence value를 1 증가시킨다.
//	m_fenceValues[m_frameIndex]++;
//}

void IdealRenderer::CreateGraphicsFence()
{
	Check(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_graphicsFence.GetAddressOf())));

	m_graphicsFenceValue = 0;

	m_graphicsFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

uint64 IdealRenderer::GraphicsFence()
{
	m_graphicsFenceValue++;
	m_commandQueue->Signal(m_graphicsFence.Get(), m_graphicsFenceValue);
	return m_graphicsFenceValue;
}

void IdealRenderer::WaitForGraphicsFenceValue()
{
	const uint64 expectedFenceValue = m_graphicsFenceValue;

	if (m_graphicsFence->GetCompletedValue() < expectedFenceValue)
	{
		m_graphicsFence->SetEventOnCompletion(expectedFenceValue, m_graphicsFenceEvent);
		WaitForSingleObject(m_graphicsFenceEvent, INFINITE);
	}
}

void IdealRenderer::GraphicsPresent()
{
	HRESULT hr = m_swapChain->Present(1, 0);
	if (DXGI_ERROR_DEVICE_REMOVED == hr)
	{
		__debugbreak();
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	GraphicsFence();
	WaitForGraphicsFenceValue();
}

void IdealRenderer::ExecuteCommandList()
{
	//ID3D12CommandList* lists[] = { m_commandList.Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	//const uint64 currentFenceValue = m_uploadFenceValue;
	//Check(m_commandQueue->Signal(m_uploadFence.Get(), currentFenceValue));

	//m_uploadFenceValue++;

	//if (m_uploadFence->GetCompletedValue() < m_uploadFenceValue)
	//{
	//	m_uploadFence->SetEventOnCompletion(m_uploadFenceValue, m_uploadFenceEvent);
	//	WaitForSingleObjectEx(m_uploadFenceEvent, INFINITE, FALSE);
	//}
}