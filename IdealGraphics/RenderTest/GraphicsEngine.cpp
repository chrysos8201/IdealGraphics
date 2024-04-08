#include "RenderTest/GraphicsEngine.h"
#include "Misc/AssimpLoader.h"
#include <DirectXColors.h>

#include "Misc/Utils/PIX.h"

GraphicsEngine::GraphicsEngine(HWND hwnd, uint32 width, uint32 height)
	: m_hwnd(hwnd)
{
	// Viewport를 만들어야겠지
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
	// 장면을 렌더링하기 위해 필요한 모든 명령을 명령 목록에 기록한다.
	// 2024.04.07 : 말하자면 Begin Render~? (케로로 모아 톤으로)
	PopulateCommandList();

	// commandList를 실행한다.
	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	Check(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void GraphicsEngine::LoadPipeline()
{
	// DXGI Factory 만들어주자.
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;

	uint32 factoryFlag = 0;

	Check(CreateDXGIFactory2(factoryFlag, IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

	// Create Adapter
	//ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
	//GetHardwar

	// 일단 하드웨어 어뎁터는 기본 설정으로 사용하자.

	// Create Device
	Check(D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,	// 일단 이걸로, dx11지원 기능이라는데 나중에 바꾸면 되지 않을까
		IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())
	));

	// Create Command Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	// 기본설정

	Check(m_device->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())
	));

	// Create SwapChain
	// CreateSwapChainForHWND가 SwapChain1밖에 지원을 안하는 것 같아서 SwapChain1로 만든다음 SwapChain3에 넣어주는듯 
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 포면 또는 리소스 출력 렌더링 대상으로 사용
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
		// 2024.04.07 사이즈 이걸 왜 얻어올까? 


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
		// GPU Heap에서 사용되는 m_rtvHeap의 첫 주소를 CPU Handle로 가져오는 것 같다.
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// RTV를 각 프레임마다 만들어준다. 
		for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].GetAddressOf()));
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);	// offset 기능은 d3dx12에 들어있고 포인터를 다음으로 넘겨주는 용도이다.
			// 2024.04.07 위에서 사이즈를 왜 구하나 했었는데 handle의 포인터를 사이즈만큼 다음응로 이동시켜주는 역할인가 보다.
			// 그래서 반복문에서 다음에 rtvHandle값은 GPU Descriptor Heap의 두 번째 rtv의 주소를 가져온다.
			// 라기 보다는 두 번째의 rtv를 가리키는 Handle이 되겠지?
		}
	}

	// 각 스레드들은 Command Allocator를 각자 가지고 있어야 한다. 
	// Command List, Command Allocator 둘 다 각자
	// Command Queue는 여러 스레드에서 공유해도 된다.
	Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
}

void GraphicsEngine::LoadAsset()
{
	// 이 밑은 빈 루트 시그니쳐를 만든다. 
	// 2024.04.07
	// 루트 시그니쳐는 쉐이더와 정보를 연결해주기 위해 만드는 것으로 기억한다.
	{
		//CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//// ID3DBlob은 메모리를 담는 용도로 사용하거나 에러메세지를 가지게 할 수 있다?
		//ComPtr<ID3DBlob> signature;
		//ComPtr<ID3DBlob> error;

		//// CreateRootSignature에 전달할 수 있는 루트 시그니쳐를 Serialization한다?
		//Check(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
		//Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	// 루트 시그니쳐를 만든다.
	// 하나의 상수 버퍼로 구성된 descriptor table을 포함하는 루트 시그니쳐를 만든다
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

		// 인풋 레이아웃에서 파이프라인 스테이트에서 필요없는 것들을 거부한다!
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

	// PSO를 만든다.
	// 쉐이더를 컴파일하고 로딩할 것이다.
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

		// vertex 정보
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

		// 이제 PSO를 만들어본다
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = iaDesc;
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());	// D3D12_SHADER_BYTECODE : 쉐이더 데이터를 포함하는 메모리 블럭에 대한 포인터이다.
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;	// 2024.04.07 : PSO 상태에서 현재 내보내는 용도로 사용될 렌더타겟이 하나라는 의미 아닐까?
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;	// 멀티 샘플링 개수이다. msaa

		Check(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf())));
	}

	// Command List를 만든다. 참고로 ID3D12CommandList를 상속받는 ID3D12GraphicsCommandList로 만들어준다.
	// 2024.04.07 : CommandList만들때 PSO를 가지고 만든다는 것은 PSO마다 하나의 CommandList가 만들어진다는 뜻인가?
	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(m_commandList.GetAddressOf())));
	// CommandList를 만들때 열린 상태로 만들어진다.
	Check(m_commandList->Close());


	// VertexBuffer를 만든다.
	{
		// 일단 삼각형만
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

		// 샘플 코드 : 
		// 주의: 정적 데이터를 전송하는 데 업로드 힙을 사용하는 것은 권장되지 않습니다.
		// GPU가 필요할 때마다 업로드 힙이 재조정됩니다.
		// 기본 힙 사용에 대해 읽어보십시오.
		// 코드 간결성과 실제로 전송해야 할 버텍스가 매우 적기 때문에 여기서는 업로드 힙을 사용하고 있습니다.
		// 2024.04.07
		// : 데이터를 GPU에 보낼때 업로드 버퍼 사용을 자제하라는건가?
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

		// vertex buffer view 초기화
		// 2024.04.07 :
		// Vertex_Buffer_view를 가지고 와서 뭐하지?
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

		// ConstantBufferView를 만든다. 
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// Map을 하고 constant buffer를 초기화한다.
		// 여기서 UnMap을 하지 않는데
		// 리소스의 수명 동안 매핑된 상태로 유지하는 것은 괜찮다.
		// 2024.04.07 :
		// 정말 괜찮을까? 샘플 코드에서는 삼각형이 하나만 있다. 다른 리소스, 모델은 없다는 건데 
		// 그래서 UnMap하고 계속 여기에 constant buffer의 정보를 업데이트 해주니까 그런거 아닐까?
		//
		CD3DX12_RANGE readRange(0, 0);
		Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
		memcpy(m_cbvDataBegin, &m_transform, sizeof(m_transform));
	}

	// 동기화 객체를 생성하고 resource가 GPU에 업로드될 때까지 기다린다.
	{
		Check(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
		m_fenceValue = 1;

		// 프레임 동기화를 위한 이벤트 핸들을 만든다.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			Check(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Command List가 execute하는 것을 기다린다.
		WaitForPreviousFrame();
	}
}

void GraphicsEngine::PopulateCommandList()
{
	// CommandAllocator는 연관된 Command List가 실행을 완료한 경우에만 재설정할 수 있다.
	// 프로그램은 GPU 실행 진행 상황을 확인하기 위해 fence를 사용해야 한다.
	Check(m_commandAllocator->Reset());

	// Command Queue로 특정 Command List를 ExecuteCommandList()로 호출되면 해당 명령 목록은 
	// 언제든지 다시 reset할 수 있으며, 다시 Command를 기록하기 전에 reset해야한다.
	Check(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);	// desciptor heap을 설정해주고
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());	// 2024.04.07 이거 뭐하는걸까?
	m_commandList->RSSetViewports(1, &m_viewport);	// viewport 설정하고
	m_commandList->RSSetScissorRects(1, &m_scissorRect);	// 가위사각형 설정하고

	// back buffer가 렌더타겟으로 사용될 것을 알려주어야 한다.
	CD3DX12_RESOURCE_BARRIER backBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferResourceBarrier);

	// CPU DescriptorHandle을 가져와서 OMSetRenderTargets를 해준다.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::AliceBlue, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);	// drawIndexInstanced가 아니라 인덱스 정보가 없다!

	// backBuffer를 이제 Present를 해야하니 state 변경을 다시 해준다
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
	// 샘플 코드 : 
	// 프레임이 완료될 때까지 대기하는 것은 최상의 방법이 아닙니다.
	// 이 코드는 간결성을 위해 이렇게 구현되었습니다. D3D12HelloFrameBuffering 샘플에서는
	// fence를 사용하여 효율적인 리소스 사용과 GPU 활용도를 최대화하는 방법을 보여줍니다.
	//
	//const uint64 fence = m_fenceValue;	
	//Check(m_commandQueue->Signal(m_fence.Get(), fence));
	//m_fenceValue++;

	//// 이전 프레임이 끝날때까지 기다린다.
	//if (m_fence->GetCompletedValue() < fence)
	//{
	//	Check(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
	//	WaitForSingleObject(m_fenceEvent, INFINITE);
	//}

	//// 2024.04.07 :
	//// 여기서 프레임 인덱스 정보를 받아주는 이유가 뭘까?
	////
	//m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();




	//const uint64 currentFenceValue = m_fenceValue[m_frameIndex];
	//Check(m_commandQueue->Signal(m_fence.Get(), currentFenceValue
}
