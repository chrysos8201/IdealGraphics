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
	m_constantBufferData.offset.x += m_offsetSpeed;
	if (m_constantBufferData.offset.x > 1.25f)
	{
		m_constantBufferData.offset.x = -1.25f;
	}
	//memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	// 2024.04.13 constantBuffer ����
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
		// CBV�� �ٽ� ��������� Root Signature�� �ٽ� �����.
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);	// vertex shader������ ���ٿ� �����ϵ����� �Ǿ��ֱ� �ѵ� hlsli�� ���谡 �������� �𸣰ڴ�. ��� ��������̶� ����� ��������

		
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
		// ComPtr�� CPU ��ü�̴�. �ٸ� �� ���ҽ��� GPU���� �������� ��� ����Ʈ�� �Ϸ�� ������ ������� �ȵȴ�.
		// �츮�� ����� �ְ� �����ϱ� �������� �� ���ҽ��� �״�� �����ؾ� �ϸ�,
		// ����� �����ߴ� �ص� �츮�� �� ����� GPU�� ���� �����ϰ� ���� ������ �𸥴�.
		// ���� �� ���� ���� ��� ������ ������ ��� ������ ���������� Wait�� �ɾ��־� �� ���ҽ� ������ 
		// �о�ְڴ�!!@!!!


		// �ƾƤ������������������������������� �� close�� �ȵǴµ�
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// ���ε� ������ ���� �ϳ��� ����� �� ����.

		VertexTest triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const uint32 vertexBufferSize = sizeof(triangleVertices);

		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);	// vertexBufferSize��ŭ Ȯ���ش޶�� ������ �ƴұ�?
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,		// ���ε� ���� COPY_DEST ���¿��� �Ѵܴ�.
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
		vertexBufferUpload->Unmap(0, nullptr);	// ��� �̰� �ý��� �޸𸮿� �ִ°Ŷ� ���� UnMap���ص� �ȴ�?

		m_commandList->CopyBufferRegion(m_vertexBuffer.Get(), 0, vertexBufferUpload.Get(), 0, vertexBufferSize);
		// �ϴ� ���ҽ� ����� ���ְ�
		CD3DX12_RESOURCE_BARRIER vertexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		m_commandList->ResourceBarrier(1, &vertexBufferBarrier);

		// vertex buffer view�� �����!
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
		m_vertexBufferView.StrideInBytes = sizeof(VertexTest);

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

		m_commandAllocators[m_frameIndex]->Reset();
		m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

		// Index Buffer�� ����ڴ�. ���⼭�� �Ƹ� wait�� �� �� ����� �ѵ� �ذ��ϴ� ������ �־�̱� ������ �ϴ��� �׳� wait�ɾ ����ڵ�.

		uint32 indices[] = { 0,1,2 };
		const uint32 indexBufferSize = sizeof(indices);

		// GPU���ۿ� ���ε� ���۸� ����ڴ�.
		ComPtr<ID3D12Resource> indexBufferUpload = nullptr;
		{
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

			// ���� �������� �����
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(m_indexBuffer.GetAddressOf())
			));
		}
		// ���ε� ���۵� �����.
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

		// ���ε���ۿ� ���� ����
		void* mappedUploadBuffer = nullptr;
		Check(indexBufferUpload->Map(0, nullptr, &mappedUploadBuffer));
		memcpy(mappedUploadBuffer, indices, indexBufferSize);
		indexBufferUpload->Unmap(0, nullptr);

		// ���ε� ���ۿ��� GPU���۷� �����ϰ� ���ҽ� ����� �Ǵ�.
		m_commandList->CopyBufferRegion(m_indexBuffer.Get(), 0, indexBufferUpload.Get(), 0, indexBufferSize);
		CD3DX12_RESOURCE_BARRIER indexBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
		);
		m_commandList->ResourceBarrier(1, &indexBufferBarrier);

		// ���� �� ���ۿ� ���� view�� �����.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_indexBufferView.SizeInBytes = indexBufferSize;

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

		// 2024.04.11 : ConstantBufferView �����
		// D3D12_HEAP_TYPE_UPLOAD�� ����Ͽ� �� �Ӽ��� �����ϸ� ���ε� ���� �����˴ϴ�. ���ε� ���� CPU���� GPU�� �����͸� ������ �� ���˴ϴ�. �̴� �Ϲ������� CPU�� �����͸� �غ��ϰ� GPU�� �����ϴ� �� ���Ǵ� �ӽ����� �޸��Դϴ�.
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

			// descibe�ϰ� cbv�� �����.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = testConstantBufferSize;
			m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

			/// Map���� ���� UnMap�� ������ �����ǵ� �ý��� �޸𸮿� �����͸� �����Ŷ� �ּҰ� �����?���� �����״� ��� ����ش�..
			// m_cbvDataBegin���� ��� ���۸� ���� �ּҸ� �����´�.
			CD3DX12_RANGE readRange(0, 0);
			Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
			// ������ �ּҺ��� �����͸� �־��ش�.
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
	// �������

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
		// CBV�� �ٽ� ��������� Root Signature�� �ٽ� �����.
		//CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);	// vertex shader������ ���ٿ� �����ϵ����� �Ǿ��ֱ� �ѵ� hlsli�� ���谡 �������� �𸣰ڴ�. ��� ��������̶� ����� ��������
		//rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
		
		// 2024.04.13 : ��Ʈ �ñ״��� ���� �ٲ㺸�ڴ�
		// 1�� �ְ�, b0�� �� ���̰�, data_static�� �Ϲ������� ��� ���� ������ �ִ� ���̶�� �Ѵ�(?), �׸��� �� �����ʹ� vertex ���̴����� ����� ���̴�
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
		ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// ���ε� ������ ���� �ϳ��� ����� �� ����.

		VertexTest triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const uint32 vertexBufferSize = sizeof(triangleVertices);

		// UploadBuffer�� �����.
		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(m_device.Get(), vertexBufferSize);

		void* mappedUploadHeap = uploadBuffer.Map();
		memcpy(mappedUploadHeap, triangleVertices, sizeof(triangleVertices));
		uploadBuffer.UnMap();

		uint32 vertexTypeSize = sizeof(VertexTest);
		uint32 vertexCount = _countof(triangleVertices);
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
		uint32 indices[] = { 0,1,2 };
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

		// 2024.04.11 : ConstantBufferView �����
		// D3D12_HEAP_TYPE_UPLOAD�� ����Ͽ� �� �Ӽ��� �����ϸ� ���ε� ���� �����˴ϴ�. ���ε� ���� CPU���� GPU�� �����͸� ������ �� ���˴ϴ�. �̴� �Ϲ������� CPU�� �����͸� �غ��ϰ� GPU�� �����ϴ� �� ���Ǵ� �ӽ����� �޸��Դϴ�.
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

			// descibe�ϰ� cbv�� �����.
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = testConstantBufferSize;
			m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			/// Map���� ���� UnMap�� ������ �����ǵ� �ý��� �޸𸮿� �����͸� �����Ŷ� �ּҰ� �����?���� �����״� ��� ����ش�..
			// m_cbvDataBegin���� ��� ���۸� ���� �ּҸ� �����´�.
			CD3DX12_RANGE readRange(0, 0);
			Check(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
			// ������ �ּҺ��� �����͸� �־��ش�.
			memcpy(m_cbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
		}

		// 2024.04.13 : ConstantBuffer �ٽ� �����.
		// ������ ���� ��ŭ �޸𸮸� �Ҵ� �� ���̴�.
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
	// �������

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

	// 2024.04.13 : ���� frameIndex�� �´� CB�� �ּҸ� �����ͼ� ���ε��Ѵ�.
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
