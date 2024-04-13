#include "IdealRenderer.h"
#include "RenderTest/VertexInfo.h"
#include <DirectXColors.h>
#include "Misc/Utils/PIX.h"


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
	//uint32 ref_count = m_device->Release();
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

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Check(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

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

	// 2024.04.11 cbv Heap을 만든다.
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
	m_constantBufferData.offset.x += m_offsetSpeed;
	if (m_constantBufferData.offset.x > 1.25f)
	{
		m_constantBufferData.offset.x = -1.25f;
	}
	//memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	// 2024.04.13 constantBuffer 수정
	m_testOffsetConstantBufferDataBegin = (TestOffset*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
	m_testOffsetConstantBufferDataBegin->offset = m_constantBufferData.offset;
}

void IdealRenderer::Render()
{
	//PopulateCommandList();
	PopulateCommandList2();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	Check(m_swapChain->Present(1, 0));
	MoveToNextFrame();
}

void IdealRenderer::LoadAsset()
{
	/*{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		Check(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}*/

	{
		// CBV를 다시 만들었으니 Root Signature을 다시 만든다.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);	// vertex shader에서만 접근에 가능하도록은 되어있긴 한데 hlsli과 관계가 있을련지 모르겠다. 사실 헤더파일이라 상관은 없을지도

		
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

		uint32 compileFlags = 0;
#if defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		Check(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	//Check(m_commandList->Close());

	{
		// 2024.04.08
		// ComPtr은 CPU 객체이다. 다만 이 리소스는 GPU에서 실행중인 명령 리스트가 완료될 때까지 사라지면 안된다.
		// 우리가 명령을 넣고 실행하기 전까지는 이 리소스는 그대로 유지해야 하며,
		// 명령을 실행했다 해도 우리는 이 명령을 GPU가 언제 시작하고 언제 끝낼지 모른다.
		// 따라서 뭐 별로 좋은 방법 같지는 않지만 명령 실행이 끝날때까지 Wait를 걸어주어 이 리소스 내용을 
		// 밀어넣겠다!!@!!!


		// 아아ㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏㅏ 왜 close가 안되는데
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// 업로드 용으로 버퍼 하나를 만드는 것 같다.

		VertexTest triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const uint32 vertexBufferSize = sizeof(triangleVertices);

		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);	// vertexBufferSize만큼 확보해달라는 정보가 아닐까?
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,		// 업로드 전은 COPY_DEST 상태여야 한단다.
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)
			));
		}

		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&vertexBufferUpload)
			));
		}
		void* mappedUploadHeap = nullptr;
		Check(vertexBufferUpload->Map(0, nullptr, &mappedUploadHeap));
		//memcpy(mappedUploadHeap, triangleVertices, sizeof(triangleVertices));
		memcpy(mappedUploadHeap, triangleVertices, sizeof(triangleVertices));
		vertexBufferUpload->Unmap(0, nullptr);	// 사실 이건 시스템 메모리에 있는거라 굳이 UnMap안해도 된다?

		m_commandList->CopyBufferRegion(m_vertexBuffer.Get(), 0, vertexBufferUpload.Get(), 0, vertexBufferSize);
		// 일단 리소스 베리어를 쳐주고
		CD3DX12_RESOURCE_BARRIER vertexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		m_commandList->ResourceBarrier(1, &vertexBufferBarrier);

		// vertex buffer view를 만든다!
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
		m_vertexBufferView.StrideInBytes = sizeof(VertexTest);

		// 자 vertexBuffer를 GPU에 복사해주어야 하니까 Wait를 일단 걸어준다.
		// command List를 일단 닫아주고 실행시킨다.
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };	// 하나밖에 없다
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

			// 동기화 오브젝트를 만들고 에셋 정보가 GPU에 업로드되기까지 기다린다.
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

		m_commandAllocators[m_frameIndex]->Reset();
		m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

		// Index Buffer도 만들겠다. 여기서도 아마 wait을 걸 것 같기는 한데 해결하는 전략이 있어보이긴 하지만 일단은 그냥 wait걸어서 만들겠따.

		uint32 indices[] = { 0,1,2 };
		const uint32 indexBufferSize = sizeof(indices);

		// GPU버퍼와 업로드 버퍼를 만들겠다.
		ComPtr<ID3D12Resource> indexBufferUpload = nullptr;
		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

			// 복사 목적지로 만들고
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(m_indexBuffer.GetAddressOf())
			));
		}
		// 업로드 버퍼도 만든다.
		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(indexBufferUpload.GetAddressOf())
			));
		}

		// 업로드버퍼에 먼저 복사
		void* mappedUploadBuffer = nullptr;
		Check(indexBufferUpload->Map(0, nullptr, &mappedUploadBuffer));
		memcpy(mappedUploadBuffer, indices, indexBufferSize);
		indexBufferUpload->Unmap(0, nullptr);

		// 업로드 버퍼에서 GPU버퍼로 복사하고 리소스 베리어를 건다.
		m_commandList->CopyBufferRegion(m_indexBuffer.Get(), 0, indexBufferUpload.Get(), 0, indexBufferSize);
		CD3DX12_RESOURCE_BARRIER indexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
		);
		m_commandList->ResourceBarrier(1, &indexBufferBarrier);

		// 이제 이 버퍼에 대한 view를 만든다.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_indexBufferView.SizeInBytes = indexBufferSize;

		// 일단 커맨드 리스트를 닫는다
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			// 여기서도 Wait를 걸면 될까?
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

		// 2024.04.11 : ConstantBufferView 만든다
		// D3D12_HEAP_TYPE_UPLOAD를 사용하여 힙 속성을 지정하면 업로드 힙이 생성됩니다. 업로드 힙은 CPU에서 GPU로 데이터를 전송할 때 사용됩니다. 이는 일반적으로 CPU가 데이터를 준비하고 GPU에 전송하는 데 사용되는 임시적인 메모리입니다.
		{
			const uint32 testConstantBufferSize = sizeof(TestOffset);

			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(testConstantBufferSize);

			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
			));

			// descibe하고 cbv를 만든다.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = testConstantBufferSize;
			m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

			/// Map으로 열고 UnMap을 해주지 않을건데 시스템 메모리에 데이터를 잡을거라 주소가 변경될?일은 없을테니 계속 잡아준다..
			// m_cbvDataBegin으로 상수 버퍼를 열고 주소를 가져온다.
			CD3DX12_RANGE readRange(0, 0);
			Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
			// 가져온 주소부터 데이터를 넣어준다.
			memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
		}
	}
}

void IdealRenderer::PopulateCommandList()
{
	Check(m_commandAllocators[m_frameIndex]->Reset());
	Check(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 2024.04.11 cb bind
	ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	// 여기까지

	m_commandList->RSSetViewports(1, &m_viewport.GetViewport());
	m_commandList->RSSetScissorRects(1, &m_viewport.GetScissorRect());

	CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferRenderTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Violet, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	m_commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	//m_commandList->DrawInstanced(3, 1, 0, 0);

	CD3DX12_RESOURCE_BARRIER backBufferPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &backBufferPresent);
	Check(m_commandList->Close());
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
		// CBV를 다시 만들었으니 Root Signature을 다시 만든다.
		//CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);	// vertex shader에서만 접근에 가능하도록은 되어있긴 한데 hlsli과 관계가 있을련지 모르겠다. 사실 헤더파일이라 상관은 없을지도
		//rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
		
		// 2024.04.13 : 루트 시그니쳐 뭔가 바꿔보겠다
		// 1개 있고, b0에 들어갈 것이고, data_static은 일반적으로 상수 버퍼 설정에 넣는 것이라고 한다(?), 그리고 이 데이터는 vertex 쉐이더에서 사용할 것이다
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);


		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		//CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1,rootParameters,0,nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

		uint32 compileFlags = 0;
#if defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		Check(D3DCompileFromFile(L"Shaders/Triangle.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		Check(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	//Check(m_commandList->Close());

	{
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// 업로드 용으로 버퍼 하나를 만드는 것 같다.

		VertexTest triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const uint32 vertexBufferSize = sizeof(triangleVertices);

		// UploadBuffer를 만든다.
		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(m_device.Get(), vertexBufferSize);

		void* mappedUploadHeap = uploadBuffer.Map();
		memcpy(mappedUploadHeap, triangleVertices, sizeof(triangleVertices));
		uploadBuffer.UnMap();

		uint32 vertexTypeSize = sizeof(VertexTest);
		uint32 vertexCount = _countof(triangleVertices);
		m_idealVertexBuffer.Create(m_device.Get(), m_commandList.Get(), vertexTypeSize, vertexCount, uploadBuffer);

		// 자 vertexBuffer를 GPU에 복사해주어야 하니까 Wait를 일단 걸어준다.
		// command List를 일단 닫아주고 실행시킨다.
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };	// 하나밖에 없다
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

			// 동기화 오브젝트를 만들고 에셋 정보가 GPU에 업로드되기까지 기다린다.
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

	// Index Buffer도 만들겠다. 여기서도 아마 wait을 걸 것 같기는 한데 해결하는 전략이 있어보이긴 하지만 일단은 그냥 wait걸어서 만들겠따.
	{
		uint32 indices[] = { 0,1,2 };
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

		// 일단 커맨드 리스트를 닫는다
		{
			Check(m_commandList->Close());
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			// 여기서도 Wait를 걸면 될까?
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

		// 2024.04.11 : ConstantBufferView 만든다
		// D3D12_HEAP_TYPE_UPLOAD를 사용하여 힙 속성을 지정하면 업로드 힙이 생성됩니다. 업로드 힙은 CPU에서 GPU로 데이터를 전송할 때 사용됩니다. 이는 일반적으로 CPU가 데이터를 준비하고 GPU에 전송하는 데 사용되는 임시적인 메모리입니다.
		{
			const uint32 testConstantBufferSize = sizeof(TestOffset);

			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(testConstantBufferSize);

			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_constantBuffer.GetAddressOf())
			));

			// descibe하고 cbv를 만든다.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = testConstantBufferSize;
			m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			/// Map으로 열고 UnMap을 해주지 않을건데 시스템 메모리에 데이터를 잡을거라 주소가 변경될?일은 없을테니 계속 잡아준다..
			// m_cbvDataBegin으로 상수 버퍼를 열고 주소를 가져온다.
			CD3DX12_RANGE readRange(0, 0);
			Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
			// 가져온 주소부터 데이터를 넣어준다.
			memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
		}

		// 2024.04.13 : ConstantBuffer 다시 만든다.
		// 프레임 개수 만큼 메모리를 할당 할 것이다.
		{
			const uint32 testConstantBufferSize = sizeof(TestOffset);

			m_idealConstantBuffer.Create(m_device.Get(), testConstantBufferSize, FRAME_BUFFER_COUNT);

			//void* beginData = m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
			//memcpy(beginData, &m_constantBufferData, sizeof(m_constantBufferData));
			m_testOffsetConstantBufferDataBegin = (TestOffset*)m_idealConstantBuffer.GetMappedMemory(m_frameIndex);
			m_testOffsetConstantBufferDataBegin->offset = m_constantBufferData.offset;
		}
	}
}

void IdealRenderer::PopulateCommandList2()
{
	Check(m_commandAllocators[m_frameIndex]->Reset());
	Check(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 2024.04.11 cb bind
	//ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
	//m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	//m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	// 여기까지

	m_commandList->RSSetViewports(1, &m_viewport.GetViewport());
	m_commandList->RSSetScissorRects(1, &m_viewport.GetScissorRect());

	CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &backBufferRenderTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Violet, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto vbView = m_idealVertexBuffer.GetView();
	m_commandList->IASetVertexBuffers(0, 1, &vbView);
	auto ibView = m_idealIndexBuffer.GetView();
	m_commandList->IASetIndexBuffer(&ibView);
	//m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());

	// 2024.04.13 : 현재 frameIndex에 맞는 CB의 주소를 가져와서 바인딩한다.
	m_commandList->SetGraphicsRootConstantBufferView(0, m_idealConstantBuffer.GetGPUVirtualAddress(m_frameIndex));

	m_commandList->DrawIndexedInstanced(m_idealIndexBuffer.GetElementCount(), 1, 0, 0, 0);
	//m_commandList->DrawInstanced(3, 1, 0, 0);

	CD3DX12_RESOURCE_BARRIER backBufferPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &backBufferPresent);
	Check(m_commandList->Close());
}

void IdealRenderer::WaitForGPU()
{
	Check(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	Check(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	m_fenceValues[m_frameIndex]++;
}
