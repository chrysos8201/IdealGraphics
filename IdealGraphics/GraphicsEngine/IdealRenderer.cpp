#include "IdealRenderer.h"
#include "RenderTest/VertexInfo.h"
#include <DirectXColors.h>
#include "Misc/Utils/PIX.h"

#include "GraphicsEngine/Mesh.h"
#include "Misc/AssimpLoader.h"

IdealRenderer::IdealRenderer(HWND hwnd, uint32 width, uint32 height)
	: m_hwnd(hwnd),
	m_width(width),
	m_height(height),
	m_viewport(hwnd, width, height)
{
	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	m_idealPipelineState = std::make_shared<Ideal::D3D12PipelineState>();
}

IdealRenderer::~IdealRenderer()
{
	//uint32 ref_count = m_device->Release();
	for (auto obj : m_objects)
	{
		obj.reset();
	}
	m_objects.clear();
}

void IdealRenderer::Init()
{
#ifdef _DEBUG
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}

#endif
	uint32 dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// 2024.04.14 // �ӽÿ� wvp�����
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

	//ComPtr<IDXGIAdapter1> hardwareAdapter;
	Check(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.GetAddressOf())));


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

	// descriptor heap ���� rtv Descriptor�� ũ��
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			Check(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);

			Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));

		}
	}

	// 2024.04.14 : dsv�� ����ڴ�. ���� descriptor heap�� �����.
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

	// 2024.04.11 cbv Heap�� �����.
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	Check(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));

	////////////////////////////////////// Load ASSET
	//LoadAsset();
	LoadAsset2();
}

void IdealRenderer::Tick()
{
	// 2024.04.14 constantBuffer ����
	//m_constantBufferDataSimpleBox
	static float rot = 0.0f;
	m_simpleBoxConstantBufferDataBegin = (SimpleBoxConstantBuffer*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
	Matrix mat = Matrix::CreateRotationY(rot) * Matrix::CreateRotationX(-rot) * m_viewProj;
	Matrix tMat = mat.Transpose();
	rot += 0.01f;
	m_simpleBoxConstantBufferDataBegin->worldViewProjection = tMat;

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		ComPtr<ID3D12PipelineState> getPipeline = m_idealPipelineState->GetPipelineState(
			Ideal::EPipelineStateInputLayout::ESimpleInputElement,
			Ideal::EPipelineStateVS::ESimpleVertexShaderVS,
			Ideal::EPipelineStatePS::ESimplePixelShaderPS2,
			Ideal::EPipelineStateRS::ERasterizerStateWireFrame
		);
		if (m_currentPipelineState != getPipeline)
		{
			m_currentPipelineState = getPipeline;
		}
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		ComPtr<ID3D12PipelineState> getPipeline = m_idealPipelineState->GetPipelineState(
			Ideal::EPipelineStateInputLayout::ESimpleInputElement,
			Ideal::EPipelineStateVS::ESimpleVertexShaderVS,
			Ideal::EPipelineStatePS::ESimplePixelShaderPS,
			Ideal::EPipelineStateRS::ERasterizerStateSolid
		);
		if (m_currentPipelineState != getPipeline)
		{
			m_currentPipelineState = getPipeline;
		}
	}


	for (auto obj : m_objects)
	{
		obj->Tick();
	}

}

void IdealRenderer::Render()
{
	//PopulateCommandList();
	BeginRender();
	PopulateCommandList2();
	EndRender();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	Check(m_swapChain->Present(1, 0));
	MoveToNextFrame();
}

void IdealRenderer::MoveToNextFrame()
{
	const uint64 currentFenceValue = m_fenceValues[m_frameIndex];
	Check(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		Check(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void IdealRenderer::LoadAsset2()
{
	{
		// CBV�� �ٽ� ��������� Root Signature�� �ٽ� �����.
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		// 2024.04.13 : ��Ʈ �ñ״��� ���� �ٲ㺸�ڴ�
		// 1�� �ְ�, b0�� �� ���̰�, data_static�� �Ϲ������� ��� ���� ������ �ִ� ���̶�� �Ѵ�(?), �׸��� �� �����ʹ� vertex ���̴����� ����� ���̴�
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);


		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	// 2024.04.15 : pso�� �̸� �����ΰ� ����ϰڴ�!
	{
		m_idealPipelineState->Create(m_device.Get(), m_rootSignature.Get());

		m_currentPipelineState = m_idealPipelineState->GetPipelineState(
			Ideal::EPipelineStateInputLayout::ESimpleInputElement,
			Ideal::EPipelineStateVS::ESimpleVertexShaderVS,
			Ideal::EPipelineStatePS::ESimplePixelShaderPS,
			Ideal::EPipelineStateRS::ERasterizerStateSolid
		).Get();
	}

	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	//CreateMeshObject("Resources/Test/window.fbx");
	CreateMeshObject("Assets/JaneD/JaneD.fbx");

	//Check(m_commandList->Close());

	{
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// ���ε� ������ ���� �ϳ��� ����� �� ����.

		const VertexTest cubeVertices[] = {
			{ { -1.0f, 1.0f, -1.0f },	{1.0f, 0.0f,0.0f,1.0f } },    // Back Top Left
			{ { 1.0f, 1.0f, -1.0f},		{0.0f, 1.0f,0.0f,1.0f } },    // Back Top Right
			{ { 1.0f, 1.0f, 1.0f},		{0.0f, 0.0f,1.0f,1.0f } },    // Front Top Right
			{ { -1.0f, 1.0f, 1.0f},		{1.0f, 1.0f,0.0f,1.0f } },    // Front Top Left

			{ { -1.0f, -1.0f, -1.0f},	{0.0f, 1.0f,1.0f,1.0f } },    // Back Bottom Left
			{ { 1.0f, -1.0f, -1.0f},	{1.0f, 0.0f,1.0f,1.0f } },    // Back Bottom Right
			{ { 1.0f, -1.0f, 1.0f},		{1.0f, 1.0f,1.0f,1.0f } },    // Front Bottom Right
			{ { -1.0f, -1.0f, 1.0f},	{0.0f, 0.0f,0.0f,1.0f } },    // Front Bottom Left
		};
		const uint32 vertexBufferSize = sizeof(cubeVertices);

		// UploadBuffer�� �����.
		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(m_device.Get(), vertexBufferSize);

		void* mappedUploadHeap = uploadBuffer.Map();
		memcpy(mappedUploadHeap, cubeVertices, sizeof(cubeVertices));
		uploadBuffer.UnMap();

		uint32 vertexTypeSize = sizeof(VertexTest);
		uint32 vertexCount = _countof(cubeVertices);
		m_idealVertexBuffer.Create(m_device.Get(), m_commandList.Get(), vertexTypeSize, vertexCount, uploadBuffer);

		// �� vertexBuffer�� GPU�� �������־�� �ϴϱ� Wait�� �ϴ� �ɾ��ش�.
		// command List�� �ϴ� �ݾ��ְ� �����Ų��.
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };	// �ϳ��ۿ� ����
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

			// ����ȭ ������Ʈ�� ����� ���� ������ GPU�� ���ε�Ǳ���� ��ٸ���.
			{
				Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
				m_fenceValues[m_frameIndex]++;

				m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (m_fenceEvent == nullptr)
				{
					Check(HRESULT_FROM_WIN32(GetLastError()));
				}

				WaitForGPU();
			}
		}
	}
	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	// Index Buffer�� ����ڴ�. ���⼭�� �Ƹ� wait�� �� �� ����� �ѵ� �ذ��ϴ� ������ �־�̱� ������ �ϴ��� �׳� wait�ɾ ����ڵ�.
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


		// ���ε���ۿ� ���� ����
		void* mappedUploadBuffer = uploadBuffer.Map();
		memcpy(mappedUploadBuffer, indices, indexBufferSize);
		uploadBuffer.UnMap();

		uint32 indexTypeSize = sizeof(uint32);
		uint32 indexCount = _countof(indices);

		m_idealIndexBuffer.Create(m_device.Get(), m_commandList.Get(), indexTypeSize, indexCount, uploadBuffer);

		// �ϴ� Ŀ�ǵ� ����Ʈ�� �ݴ´�
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			// ���⼭�� Wait�� �ɸ� �ɱ�?
			{
				Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
				m_fenceValues[m_frameIndex]++;

				m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (m_fenceEvent == nullptr)
				{
					Check(HRESULT_FROM_WIN32(GetLastError()));
				}

				WaitForGPU();
			}
		}

		// 2024.04.13 : ConstantBuffer �ٽ� �����.
		// ������ ���� ��ŭ �޸𸮸� �Ҵ� �� ���̴�.
		{
			const uint32 testConstantBufferSize = sizeof(SimpleBoxConstantBuffer);

			m_idealConstantBuffer.Create(m_device.Get(), testConstantBufferSize, FRAME_BUFFER_COUNT);

			//void* beginData = m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
			//memcpy(beginData, &m_constantBufferData, sizeof(m_constantBufferData));
			m_simpleBoxConstantBufferDataBegin = (SimpleBoxConstantBuffer*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
			//m_testOffsetConstantBufferDataBegin->offset = m_constantBufferData.offset;
		}
	}
}

void IdealRenderer::ExecuteCommandList()
{
	Check(m_commandList->Close());
	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	// ���⼭�� Wait�� �ɸ� �ɱ�?
	{
		Check(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			Check(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForGPU();
	}
	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
}

void IdealRenderer::CreateMeshObject(const std::string Path)
{
	//std::vector<Vertex> vertices;
	//std::vector<uint32> indices;
	
	//std::vector<std::shared_ptr<Ideal::Mesh>> meshes;

	AssimpLoader assimpLoader;

	ImportSettings3 setting = { Path, m_objects };
	
	assimpLoader.Load(setting);

	for (auto& mesh : m_objects)
	{
		//IdealRenderer* renderer = shared_from_this().get();
		mesh->Create(shared_from_this());
		//m_objects.push_back(mesh);
	}
	//std::shared_ptr<Ideal::Mesh> mesh = std::make_shared<Ideal::Mesh>();
	//mesh->Create(shared_from_this());
	
	//m_objects.push_back()
}

void IdealRenderer::BeginRender()
{
	Check(m_commandAllocators[m_frameIndex]->Reset());

	// 2024.04.15 pipeline state�� m_currentPipelineState�� �ִ� ������ �����Ѵ�.
	Check(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

	m_commandList->RSSetViewports(1, &m_viewport.GetViewport());
	m_commandList->RSSetScissorRects(1, &m_viewport.GetScissorRect());

	CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferRenderTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	// 2024.04.14 dsv����
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Violet, 0, nullptr);
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

std::shared_ptr<Ideal::D3D12PipelineState> IdealRenderer::GetPipelineStates()
{
	return m_idealPipelineState;
}

void IdealRenderer::PopulateCommandList2()
{
	
	m_commandList->SetPipelineState(m_currentPipelineState.Get());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto vbView = m_idealVertexBuffer.GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbView);
	auto ibView = m_idealIndexBuffer.GetView();
	m_commandList->IASetIndexBuffer(&ibView);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	// 2024.04.13 : ���� frameIndex�� �´� CB�� �ּҸ� �����ͼ� ���ε��Ѵ�.
	m_commandList->SetGraphicsRootConstantBufferView(0, m_idealConstantBuffer.GetGPUVirtualAddress(m_frameIndex));

	m_commandList->DrawIndexedInstanced(m_idealIndexBuffer.GetElementCount(), 1, 0, 0, 0);
	for (auto obj : m_objects)
	{
		obj->Render(m_commandList.Get());
	}
}

void IdealRenderer::WaitForGPU()
{
	Check(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	Check(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	m_fenceValues[m_frameIndex]++;
}
