#include "RenderTest/GraphicsEngine.h"
#include "Misc/AssimpLoader.h"
#include <DirectXColors.h>

#include "Misc/Utils/PIX.h"

GraphicsEngine::GraphicsEngine(HWND hwnd, uint32 width, uint32 height)
	: m_hwnd(hwnd)
{
	// Viewport�� �����߰���
	m_viewport = CD3DX12_VIEWPORT(
		0.f,
		0.f,
		static_cast<float>(width),
		static_cast<float>(height)
	);

	// Scissor Rect
	m_scissorRect = CD3DX12_RECT(
		0.f,
		0.f,
		static_cast<float>(width),
		static_cast<float>(height)
	);

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	Vector3 eyePos(0.f, 0.f, 5.f);
	Vector3 targetPos = Vector3::Zero;
	Vector3 upward(0.f, 1.f, 0.f);

	float fov = DirectX::XMConvertToRadians(37.5f);

	m_transform.World = Matrix::Identity;
	m_transform.View = Matrix::CreateLookAt(eyePos, targetPos, upward);
	m_transform.Proj = Matrix::CreatePerspectiveFieldOfView(fov, m_aspectRatio, 0.3f, 1000.f);
	
}

GraphicsEngine::~GraphicsEngine()
{

}

void GraphicsEngine::Init()
{
#ifdef _DEBUG
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}
#endif

	LoadPipeline();
	LoadAsset();
}

void GraphicsEngine::BeginRender()
{

}

void GraphicsEngine::EndRender()
{

}

void GraphicsEngine::Tick()
{
	m_rotateY += 0.02f;
	m_transform.World.CreateRotationY(m_rotateY);

	memcpy(m_cbvDataBegin, &m_transform, sizeof(m_transform));
}

void GraphicsEngine::Render()
{
	// ����� �������ϱ� ���� �ʿ��� ��� ����� ��� ��Ͽ� ����Ѵ�.
	// 2024.04.07 : �����ڸ� Begin Render~? (�ɷη� ��� ������)
	PopulateCommandList();

	// commandList�� �����Ѵ�.
	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	Check(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void GraphicsEngine::LoadPipeline()
{
	// DXGI Factory ���������.
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;

	uint32 factoryFlag = 0;

	Check(CreateDXGIFactory2(factoryFlag, IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

	// Create Adapter
	//ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
	//GetHardwar

	// �ϴ� �ϵ���� ��ʹ� �⺻ �������� �������.

	// Create Device
	Check(D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,	// �ϴ� �̰ɷ�, dx11���� ����̶�µ� ���߿� �ٲٸ� ���� ������
		IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())
	));

	// Create Command Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// �⺻����

	Check(m_device->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())
	));

	// Create SwapChain
	// CreateSwapChainForHWND�� SwapChain1�ۿ� ������ ���ϴ� �� ���Ƽ� SwapChain1�� ������� SwapChain3�� �־��ִµ� 
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ���� �Ǵ� ���ҽ� ��� ������ ������� ���
	swapChainDesc.BufferCount = GraphicsEngine::FRAME_BUFFER_COUNT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain = nullptr;
	Check(dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		swapChain.GetAddressOf()
	));
	swapChain.As(&m_swapChain);

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create Descriptor Heap
	{
		// rtv
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		ZeroMemory(&rtvHeapDesc, sizeof(rtvHeapDesc));

		rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		Check(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.ReleaseAndGetAddressOf())));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		// 2024.04.07 ������ �̰� �� ���ñ�? 


		// cbv
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		ZeroMemory(&cbvHeapDesc, sizeof(cbvHeapDesc));

		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		Check(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));
	}

	// Create frame resources
	{
		// GPU Heap���� ���Ǵ� m_rtvHeap�� ù �ּҸ� CPU Handle�� �������� �� ����.
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// RTV�� �� �����Ӹ��� ������ش�. 
		for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].GetAddressOf()));
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);	// offset ����� d3dx12�� ����ְ� �����͸� �������� �Ѱ��ִ� �뵵�̴�.
			// 2024.04.07 ������ ����� �� ���ϳ� �߾��µ� handle�� �����͸� �����ŭ �������� �̵������ִ� �����ΰ� ����.
			// �׷��� �ݺ������� ������ rtvHandle���� GPU Descriptor Heap�� �� ��° rtv�� �ּҸ� �����´�.
			// ��� ���ٴ� �� ��°�� rtv�� ����Ű�� Handle�� �ǰ���?
		}
	}

	// �� ��������� Command Allocator�� ���� ������ �־�� �Ѵ�. 
	// Command List, Command Allocator �� �� ����
	// Command Queue�� ���� �����忡�� �����ص� �ȴ�.
	Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
}

void GraphicsEngine::LoadAsset()
{
	// �� ���� �� ��Ʈ �ñ״��ĸ� �����. 
	// 2024.04.07
	// ��Ʈ �ñ״��Ĵ� ���̴��� ������ �������ֱ� ���� ����� ������ ����Ѵ�.
	{
		//CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//// ID3DBlob�� �޸𸮸� ��� �뵵�� ����ϰų� �����޼����� ������ �� �� �ִ�?
		//ComPtr<ID3DBlob> signature;
		//ComPtr<ID3DBlob> error;

		//// CreateRootSignature�� ������ �� �ִ� ��Ʈ �ñ״��ĸ� Serialization�Ѵ�?
		//Check(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
		//Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	// ��Ʈ �ñ״��ĸ� �����.
	// �ϳ��� ��� ���۷� ������ descriptor table�� �����ϴ� ��Ʈ �ñ״��ĸ� �����
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameter[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameter[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		// ��ǲ ���̾ƿ����� ���������� ������Ʈ���� �ʿ���� �͵��� �ź��Ѵ�!
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;// |
			//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		/*CD3DX12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = std::size(rootParameter);*/

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameter), rootParameter, 0, nullptr, rootSignatureFlags);
		
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	// PSO�� �����.
	// ���̴��� �������ϰ� �ε��� ���̴�.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

		ComPtr<ID3DBlob> errormessage;

		uint32 compileFlags = 0;
#if defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		compileFlags = 0;
#endif
		//Check(D3DCompileFromFile(L"Shaders/BoxVS.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, NULL, vertexShader.GetAddressOf(), errormessage.GetAddressOf()));
		//char* errors = (char*)errormessage->GetBufferPointer();
		//Check(D3DCompileFromFile(L"Shaders/BoxVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", compileFlags, NULL, vertexShader.GetAddressOf(), errormessage.GetAddressOf()));
		//Check(D3DCompileFromFile(L"Shaders/BoxPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", compileFlags, NULL, pixelShader.GetAddressOf(), nullptr));
		
		//Check(D3DReadFileToBlob(L"../x64/Debug/BoxVS.cso", vertexShader.GetAddressOf()));
		//Check(D3DReadFileToBlob(L"../x64/Debug/BoxPS.cso", pixelShader.GetAddressOf()));
		//Check(D3DReadFileToBlob(L"../x64/Debug/Triangle.cso", vertexShader.GetAddressOf()));
		//Check(D3DReadFileToBlob(L"../x64/Debug/Triangle.cso", pixelShader.GetAddressOf()));

		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", compileFlags, NULL, vertexShader.GetAddressOf(), nullptr));
		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", compileFlags, NULL, pixelShader.GetAddressOf(), nullptr));

		// vertex ����
		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// IA
		D3D12_INPUT_LAYOUT_DESC iaDesc = {};
		iaDesc.NumElements = _countof(inputElementDesc);
		iaDesc.pInputElementDescs = inputElementDesc;

		// ���� PSO�� ������
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = iaDesc;
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());	// D3D12_SHADER_BYTECODE : ���̴� �����͸� �����ϴ� �޸� ���� ���� �������̴�.
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;	// 2024.04.07 : PSO ���¿��� ���� �������� �뵵�� ���� ����Ÿ���� �ϳ���� �ǹ� �ƴұ�?
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;	// ��Ƽ ���ø� �����̴�. msaa

		Check(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf())));
	}

	// Command List�� �����. ����� ID3D12CommandList�� ��ӹ޴� ID3D12GraphicsCommandList�� ������ش�.
	// 2024.04.07 : CommandList���鶧 PSO�� ������ ����ٴ� ���� PSO���� �ϳ��� CommandList�� ��������ٴ� ���ΰ�?
	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(m_commandList.GetAddressOf())));
	// CommandList�� ���鶧 ���� ���·� ���������.
	Check(m_commandList->Close());


	// VertexBuffer�� �����.
	{
		// �ϴ� �ﰢ����
		/*Vertex triangleVertices[] =
		{
			{Vector3(0.f, 0.25f * m_aspectRatio, 0.f), Vector3(1.f), Vector2(1.f), Vector3(1.f), Vector4(1.f)},
			{Vector3(0.f, 0.25f * m_aspectRatio, 0.f), Vector3(1.f), Vector2(1.f), Vector3(1.f), Vector4(1.f)},
			{Vector3(0.f, 0.25f * m_aspectRatio, 0.f), Vector3(1.f), Vector2(1.f), Vector3(1.f), Vector4(1.f)}
		};*/

		VertexTest triangleVertices[] =
		{
			 { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const uint32 vertexBufferSize = sizeof(triangleVertices);

		// ���� �ڵ� : 
		// ����: ���� �����͸� �����ϴ� �� ���ε� ���� ����ϴ� ���� ������� �ʽ��ϴ�.
		// GPU�� �ʿ��� ������ ���ε� ���� �������˴ϴ�.
		// �⺻ �� ��뿡 ���� �о�ʽÿ�.
		// �ڵ� ���Ἲ�� ������ �����ؾ� �� ���ؽ��� �ſ� ���� ������ ���⼭�� ���ε� ���� ����ϰ� �ֽ��ϴ�.
		// 2024.04.07
		// : �����͸� GPU�� ������ ���ε� ���� ����� �����϶�°ǰ�?
		CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		Check(m_device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())
		));

		uint8* vertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		Check(m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataBegin)));
		memcpy(vertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// vertex buffer view �ʱ�ȭ
		// 2024.04.07 :
		// Vertex_Buffer_view�� ������ �ͼ� ������?
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexTest);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// Create Constant Buffer
	{
		const uint32 constantBufferSize = sizeof(Transform);
		CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
		Check(m_device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
		));

		// ConstantBufferView�� �����. 
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// Map�� �ϰ� constant buffer�� �ʱ�ȭ�Ѵ�.
		// ���⼭ UnMap�� ���� �ʴµ�
		// ���ҽ��� ���� ���� ���ε� ���·� �����ϴ� ���� ������.
		// 2024.04.07 :
		// ���� ��������? ���� �ڵ忡���� �ﰢ���� �ϳ��� �ִ�. �ٸ� ���ҽ�, ���� ���ٴ� �ǵ� 
		// �׷��� UnMap�ϰ� ��� ���⿡ constant buffer�� ������ ������Ʈ ���ִϱ� �׷��� �ƴұ�?
		//
		CD3DX12_RANGE readRange(0, 0);
		Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
		memcpy(m_cbvDataBegin, &m_transform, sizeof(m_transform));
	}

	// ����ȭ ��ü�� �����ϰ� resource�� GPU�� ���ε�� ������ ��ٸ���.
	{
		Check(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
		m_fenceValue = 1;

		// ������ ����ȭ�� ���� �̺�Ʈ �ڵ��� �����.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			Check(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Command List�� execute�ϴ� ���� ��ٸ���.
		WaitForPreviousFrame();
	}
}

void GraphicsEngine::PopulateCommandList()
{
	// CommandAllocator�� ������ Command List�� ������ �Ϸ��� ��쿡�� �缳���� �� �ִ�.
	// ���α׷��� GPU ���� ���� ��Ȳ�� Ȯ���ϱ� ���� fence�� ����ؾ� �Ѵ�.
	Check(m_commandAllocator->Reset());

	// Command Queue�� Ư�� Command List�� ExecuteCommandList()�� ȣ��Ǹ� �ش� ��� ����� 
	// �������� �ٽ� reset�� �� ������, �ٽ� Command�� ����ϱ� ���� reset�ؾ��Ѵ�.
	Check(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);	// desciptor heap�� �������ְ�
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());	// 2024.04.07 �̰� ���ϴ°ɱ�?
	m_commandList->RSSetViewports(1, &m_viewport);	// viewport �����ϰ�
	m_commandList->RSSetScissorRects(1, &m_scissorRect);	// �����簢�� �����ϰ�

	// back buffer�� ����Ÿ������ ���� ���� �˷��־�� �Ѵ�.
	CD3DX12_RESOURCE_BARRIER backBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferResourceBarrier);

	// CPU DescriptorHandle�� �����ͼ� OMSetRenderTargets�� ���ش�.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::AliceBlue, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);	// drawIndexInstanced�� �ƴ϶� �ε��� ������ ����!

	// backBuffer�� ���� Present�� �ؾ��ϴ� state ������ �ٽ� ���ش�
	CD3DX12_RESOURCE_BARRIER presentBarrier= CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_commandList->ResourceBarrier(1, &presentBarrier);
	Check(m_commandList->Close());
}

void GraphicsEngine::WaitForPreviousFrame()
{
	// ���� �ڵ� : 
	// �������� �Ϸ�� ������ ����ϴ� ���� �ֻ��� ����� �ƴմϴ�.
	// �� �ڵ�� ���Ἲ�� ���� �̷��� �����Ǿ����ϴ�. D3D12HelloFrameBuffering ���ÿ�����
	// fence�� ����Ͽ� ȿ������ ���ҽ� ���� GPU Ȱ�뵵�� �ִ�ȭ�ϴ� ����� �����ݴϴ�.
	//
	//const uint64 fence = m_fenceValue;	
	//Check(m_commandQueue->Signal(m_fence.Get(), fence));
	//m_fenceValue++;

	//// ���� �������� ���������� ��ٸ���.
	//if (m_fence->GetCompletedValue() < fence)
	//{
	//	Check(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
	//	WaitForSingleObject(m_fenceEvent, INFINITE);
	//}

	//// 2024.04.07 :
	//// ���⼭ ������ �ε��� ������ �޾��ִ� ������ ����?
	////
	//m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();




	//const uint64 currentFenceValue = m_fenceValue[m_frameIndex];
	//Check(m_commandQueue->Signal(m_fence.Get(), currentFenceValue
}
