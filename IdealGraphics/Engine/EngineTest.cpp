#include "Engine/EngineTest.h"
#include <cassert>


Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

	// Create the actual default buffer resource.
	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf()));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &resourceBarrier);

	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &resourceBarrier);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}


EngineTest::EngineTest(HWND hwnd)
	: m_hMainWnd(hwnd)
{

}

EngineTest::~EngineTest()
{

}

void EngineTest::Init()
{
	InitD3D();
	CreateCommandObjects();
	CreateSwapChain();
	CreateRTVAndDSVDescriptorHeaps();
	CreateRTV();
	CreateDSV();
	//SetViewport();
	//SetScissorRect();

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	//CreateVertexBuffer();
}

void EngineTest::Tick()
{

}

void EngineTest::Render()
{
	Draw();
}

void EngineTest::End()
{

}

void EngineTest::InitD3D()
{
	// DXGIFactory�� �����.
	CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));


	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,	// �⺻ �����
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_d3dDevice)
	);

	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

		D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3dDevice)
		);
	}

	HRESULT hr;

	// ��Ÿ�� ��ü�� �����.
	hr = m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	if (FAILED(hr)) assert(false);

	// desc ũ�⸦ ��´�.
	m_RTVDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CBVSRVDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 4X MSAA ǰ�� ���� ���� ����
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_backBufferFormat;	// FORMAT_R8G8B8A8_UNORM
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;

	hr = m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	);
	if (FAILED(hr)) assert(false);

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA qulity level.");

}

void EngineTest::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()));
	if (FAILED(hr)) assert(false);

	hr = m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_directCmdListAlloc.GetAddressOf()));
	if (FAILED(hr)) assert(false);

	hr = m_d3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_directCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_commandList.GetAddressOf())
	);
	if (FAILED(hr)) assert(false);

	// ���� ���·� �����Ѵ�. ���� ��� ����� ó�� ������ �� Reset�� ȣ���ϴµ� Reset�� ȣ���Ϸ��� command List �� �����־�� �ϱ� �����̴�.
	m_commandList->Close();
}

void EngineTest::CreateSwapChain()
{
	m_swapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = m_dxgiFactory->CreateSwapChain(
		m_commandQueue.Get(),
		&sd,
		m_swapChain.GetAddressOf()
	);
	if (FAILED(hr)) assert(false);
}

void EngineTest::CreateRTVAndDSVDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	HRESULT hr = m_d3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf())
	);
	if (FAILED(hr)) assert(false);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	hr = m_d3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DSVHeap.GetAddressOf())
	);
	if (FAILED(hr)) assert(false);
}

void EngineTest::CreateRTV()
{
	HRESULT hr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint8 i = 0; i < SwapChainBufferCount; i++)
	{
		// ����ü���� i��° ���۸� ��´�.
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i]));
		if (FAILED(hr)) assert(false);

		// �� ���ۿ� ���� RTV�� �����Ѵ�.
		m_d3dDevice->CreateRenderTargetView(
			m_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle
		);

		// ���� ���� �׸����� �Ѿ��
		rtvHeapHandle.Offset(1, m_RTVDescriptorSize);
	}
}

void EngineTest::CreateDSV()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = m_depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = m_d3dDevice->CreateCommittedResource(
		//&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),	// 
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())
	);
	if (FAILED(hr)) assert(false);

	m_d3dDevice->CreateDepthStencilView(
		m_depthStencilBuffer.Get(),
		nullptr,
		DepthStencilView()
	);

	// �ڿ��� �ʱ� ���¿��� ���� ���۷� ����� �� �ִ� ���·� �����Ѵ�.
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,	// ���� ������Ʈ
		D3D12_RESOURCE_STATE_DEPTH_WRITE	// ���� ������Ʈ
	);

	m_commandList->ResourceBarrier(
		1,
		&resourceBarrier
	);
}

void EngineTest::SetViewport()
{
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	vp.Width = static_cast<float>(m_clientWidth);
	vp.Height = static_cast<float>(m_clientHeight);
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;

	m_commandList->RSSetViewports(1, &vp);
}

void EngineTest::SetScissorRect()
{
	// TEMP : 2��и鸸 ����
	//m_scissorRect = { 0,0, m_clientWidth / 2, m_clientHeight / 2 };
	m_scissorRect = { 0,0, m_clientWidth, m_clientHeight };
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
}

void EngineTest::Draw()
{
	HRESULT hr;
	// �缳���� GPU�� ���� ��� ��ϵ��� ��� ó���� �Ŀ� �Ͼ��.
	hr = m_directCmdListAlloc->Reset();
	if (FAILED(hr)) assert(false);
	// ��� ����� ExecuteCommandList�� ���ؼ� ��� ��⿭�� �߰��ߴٸ� ��� ����� �缳���� �� �ִ�. ��� ����� �缳���ϸ� �޸𸮰� ��Ȱ��ȴ�.
	hr = m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr);
	if (FAILED(hr)) assert(false);

	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�. ����Ÿ������ ���ڴ�!
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	m_commandList->ResourceBarrier(
		1,
		&resourceBarrier
	);

	// ����Ʈ�� ���� ���簢���� �����Ѵ�.
	// ��� ����� �缳���� ������ �̵鵵 �缳���ؾ� �Ѵ�.
	SetViewport();
	SetScissorRect();

	// �ĸ� ���ۿ� ���� ���۸� �����.
	m_commandList->ClearRenderTargetView(
		CurrentBackBufferView(),
		DirectX::Colors::LightSteelBlue, 0, nullptr
	);

	m_commandList->ClearDepthStencilView(
		DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr
	);

	// ������ ����� ��ϵ� ���� ��� ���۵��� �����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthStencilView();
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = CurrentBackBufferView();
	m_commandList->OMSetRenderTargets(1, &backBufferView, true, &dsv);



	{
		/////////////////////////////
		// Draw BOX

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = m_boxGeometry->VertexBufferView();
		m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		
		D3D12_INDEX_BUFFER_VIEW indexBufferView = m_boxGeometry->IndexBufferView();
		m_commandList->IASetIndexBuffer(&indexBufferView);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_commandList->SetGraphicsRootDescriptorTable(
			0,
			m_cbvHeap->GetGPUDescriptorHandleForHeapStart()
		);

		m_commandList->DrawIndexedInstanced(
			m_boxGeometry->DrawArgs["box"].IndexCount,
			1, 0, 0, 0
		);
	}


	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�. Present�ϰڴ�!
	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_commandList->ResourceBarrier(
		1,
		&resourceBarrier
	);


	// ��ɵ��� ����� ��ģ��.
	m_commandList->Close();

	// ��� ������ ���� ��� ����� ��� ��⿭�� �߰��Ѵ�.
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ĸ� ���ۿ� ���� ���۸� ��ȯ�Ѵ�.
	hr = m_swapChain->Present(0, 0);
	if (FAILED(hr)) assert(false);

	m_currentBackBuffer = (m_currentBackBuffer + 1) % SwapChainBufferCount;

	// �� �������� ��ɵ��� ��� ó���Ǳ� ��ٸ���. �̷��� ���� ��ȿ�����̴�.
	FlushCommendQueue();
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineTest::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),	// ù �ڵ�
		m_currentBackBuffer,								// ������ ����
		m_RTVDescriptorSize									// �������� ����Ʈ ũ��
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineTest::DepthStencilView() const
{
	return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* EngineTest::CurrentBackBuffer() const
{
	return m_swapChainBuffer[m_currentBackBuffer].Get();
}

void EngineTest::FlushCommendQueue()
{
	// ���� ��Ÿ�� ���������� ��ɵ��� ǥ���ϵ��� ��Ÿ�� ���� ������Ų��.
	m_currentFence++;

	// �� ��Ÿ�� ������ �����ϴ� ���(Signal)�� ��� ��⿭�� �߰��Ѵ�.
	// ���� �츮�� GPU Timeline�� �����Ƿ�, �� ��Ÿ�� ������ GPU�� �� Signal()
	// ��ɱ����� ��� ����� ó���ϱ� �������� �������� �ʴ´�.
	m_commandQueue->Signal(m_fence.Get(), m_currentFence);

	// GPU�� �� ��Ÿ�� ���������� ��ɵ��� �Ϸ��� ������ ��ٸ���.
	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		//HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
		//HANDLE eventHandle = CreateEvent(nullptr, false, false, EVENT_ALL_ACCESS);

		// GPU�� ���� ��Ÿ�� ������ ���������� �̺�Ʈ�� �ߵ��Ѵ�.
		m_fence->SetEventOnCompletion(m_currentFence, eventHandle);

		// GPU�� ���� ��Ÿ�� ������ ���������� ���ϴ� �̺�Ʈ�� ��ٸ���.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void EngineTest::BuildBoxGeometry()
{
	// Vertex�� Index ����
	std::array<Vertex1, 8> vertices;
	vertices[0] = { Vector3(-1.f,-1.f,-1.f), Vector4(DirectX::Colors::White) };
	vertices[1] = { Vector3(-1.f, 1.f,-1.f), Vector4(DirectX::Colors::Black) };
	vertices[2] = { Vector3(1.f, 1.f,-1.f), Vector4(DirectX::Colors::Red) };
	vertices[3] = { Vector3(1.f,-1.f,-1.f), Vector4(DirectX::Colors::Green) };
	vertices[4] = { Vector3(-1.f,-1.f, 1.f), Vector4(DirectX::Colors::Blue) };
	vertices[5] = { Vector3(-1.f, 1.f, 1.f), Vector4(DirectX::Colors::Yellow) };
	vertices[6] = { Vector3(1.f, 1.f, 1.f), Vector4(DirectX::Colors::Cyan) };
	vertices[7] = { Vector3(1.f,-1.f, 1.f), Vector4(DirectX::Colors::Magenta) };

	std::array<uint16, 36> indices = {
		0,1,2,
		0,2,3,

		4,6,5,
		4,7,6,

		4,5,1,
		4,1,0,

		3,2,6,
		3,6,7,

		1,5,6,
		1,6,2,

		4,0,3,
		4,3,7
	};

	const uint32 vbByteSize = (uint32)vertices.size() * sizeof(Vertex1);
	const uint32 ibByteSize = (uint32)indices.size() * sizeof(uint16);

	m_boxGeometry = std::make_unique<MeshGeometry>();


	D3DCreateBlob(vbByteSize, &m_boxGeometry->VertexBufferCPU);
	CopyMemory(m_boxGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	D3DCreateBlob(ibByteSize, &m_boxGeometry->IndexBufferCPU);
	CopyMemory(m_boxGeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_boxGeometry->VertexBufferGPU = CreateDefaultBuffer(
		m_d3dDevice.Get(), m_commandList.Get(), vertices.data(), vbByteSize, m_boxGeometry->VertexBufferUploader
	);

	m_boxGeometry->IndexBufferGPU = CreateDefaultBuffer(
		m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, m_boxGeometry->IndexBufferUploader
	);

	m_boxGeometry->VertexByteStride = sizeof(Vertex1);
	m_boxGeometry->VertexBufferByteSize = vbByteSize;
	m_boxGeometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_boxGeometry->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (uint32)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_boxGeometry->DrawArgs["box"] = submesh;
}

void EngineTest::CreateVertexBuffer()
{
	const uint64 byteSize = sizeof(Vertex1) * 8;
	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	//VertexBufferGPU = 
}

void EngineTest::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
}

void EngineTest::BuildConstantBuffers()
{
	m_objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 1, true);
	uint32 objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectCB->Resource()->GetGPUVirtualAddress();

	// ���ۿ��� i ��° ��ü�� ��� ������ �������� ��´�. ���� i = 0 �̴�.
	int32 boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_d3dDevice->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void EngineTest::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(),
		errorBlob.GetAddressOf()
	);

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	if (FAILED(hr)) assert(false);

	m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
}

void EngineTest::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;
	m_vsByteCode = d3dUtil::CompileShader(L"Shaders/BoxVS.hlsl", nullptr, "VS", "vs_5_0");
	m_vsByteCode = d3dUtil::CompileShader(L"Shaders/BoxPS.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void EngineTest::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (uint32)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaState - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;
	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO));
}
