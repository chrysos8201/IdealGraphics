#include "Engine/TestGraphics.h"
#include <DirectXColors.h>

#include "Engine/Engine.h"

TestGraphics::TestGraphics(HWND hwnd, uint32 width, uint32 height)
	: m_hwnd(hwnd),
	m_frameBufferWidth(width),
	m_frameBufferHeight(height)
{

}

TestGraphics::~TestGraphics()
{

}

void TestGraphics::Init()
{
	bool isSucceed = true;
	isSucceed = CreateDevice();
	assert(isSucceed);
	isSucceed = CreateCommandQueue();
	assert(isSucceed);
	isSucceed = CreateSwapChain();
	assert(isSucceed);
	isSucceed = CreateCommandList();
	assert(isSucceed);
	isSucceed = CreateFence();
	assert(isSucceed);
	CreateViewport();
	CreateScissorRect();
	CreateRenderTarget();
	CreateDepthStencil();
}

void TestGraphics::Tick()
{

}

void TestGraphics::Render()
{

}

void TestGraphics::BeginRender()
{
	m_currentRenderTarget = m_renderTargets[m_currentBackBufferIndex].Get();

	m_allocator[m_currentBackBufferIndex]->Reset();
	m_commandList->Reset(m_allocator[m_currentBackBufferIndex].Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE currentRTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	currentRTVHandle.ptr += m_currentBackBufferIndex * m_rtvDescriptorSize;

	D3D12_CPU_DESCRIPTOR_HANDLE currentDSVHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->OMSetRenderTargets(1, &currentRTVHandle, FALSE, &currentDSVHandle);

	//m_commandList->ClearRenderTargetView(currentRTVHandle, DirectX::Colors::AliceBlue, 0, nullptr);
	m_commandList->ClearRenderTargetView(currentRTVHandle, DirectX::Colors::GreenYellow, 0, nullptr);
	m_commandList->ClearDepthStencilView(currentDSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void TestGraphics::EndRender()
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	// 명령들 실행
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commendQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	HRESULT hr = m_swapChain->Present(1, 0);
	if (FAILED(hr)) assert(false);
	WaitRender();

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12Device6* TestGraphics::GetDevice()
{
	return m_device.Get();
}

ID3D12GraphicsCommandList* TestGraphics::GetCommandList()
{
	return m_commandList.Get();
}

uint32 TestGraphics::CurrentBackBufferIndex()
{
	return m_currentBackBufferIndex;
}

bool TestGraphics::CreateDevice()
{
	HRESULT hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())
	);
	return SUCCEEDED(hr);
}

bool TestGraphics::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = m_device->CreateCommandQueue(
		&desc,
		IID_PPV_ARGS(m_commendQueue.ReleaseAndGetAddressOf())
	);

	return SUCCEEDED(hr);
}

bool TestGraphics::CreateSwapChain()
{
	ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	// 스왑 체인 생성
	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferDesc.Width = m_frameBufferWidth;
	desc.BufferDesc.Height = m_frameBufferHeight;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAME_BUFFER_COUNT;
	desc.OutputWindow = m_hwnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain> swapChain = nullptr;
	hr = dxgiFactory->CreateSwapChain(m_commendQueue.Get(), &desc, &swapChain);
	if (FAILED(hr))
	{
		return false;
	}

	// SwapChain3 생성
	hr = swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	return true;
}

bool TestGraphics::CreateCommandList()
{
	HRESULT hr;
	for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		hr = m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_allocator[i].ReleaseAndGetAddressOf())
		);
	}
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_allocator[m_currentBackBufferIndex].Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)
	);
	if (FAILED(hr))
	{
		return false;
	}

	// commandList는 열린 상태로 생성되니 닫아야한다.
	m_commandList->Close();

	return true;
}

bool TestGraphics::CreateFence()
{
	//ZeroMemory(m_fenceValue, sizeof(uint64) * FRAME_BUFFER_COUNT);
	for (auto i = 0U; i < FRAME_BUFFER_COUNT; i++)
	{
		m_fenceValue[i] = 0;
	}
	HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	m_fenceValue[m_currentBackBufferIndex]++;

	// 동기화 할때의 이벤트 핸들러
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return (m_fenceEvent != nullptr);
}

void TestGraphics::CreateViewport()
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_frameBufferWidth);
	m_viewport.Height = static_cast<float>(m_frameBufferHeight);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
}

void TestGraphics::CreateScissorRect()
{
	m_scissorRect.left = 0;
	m_scissorRect.right = m_frameBufferWidth;
	m_scissorRect.top = 0;
	m_scissorRect.bottom = m_frameBufferHeight;
}

bool TestGraphics::CreateRenderTarget()
{
	// RTV용 서술자 힙 만들기
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FRAME_BUFFER_COUNT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_RTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}
	// 서술자 크기 얻기
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
		//rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	// -> 위의 CD3DX12_CPU_DESCRIPTOR_HANDLE은 편의성을 위한 d3dx12의 함수이다!!!

	return true;
}

bool TestGraphics::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DSVHeap));
	if (FAILED(hr))
	{
		return false;
	}
	m_DSVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_CLEAR_VALUE dsvClearValue;
	dsvClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsvClearValue.DepthStencil.Depth = 1.0f;
	dsvClearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		m_frameBufferWidth,
		m_frameBufferHeight,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
	);

	hr = m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClearValue,
		IID_PPV_ARGS(m_depthStencilBuffer.ReleaseAndGetAddressOf())
	);

	if (FAILED(hr))
	{
		return false;
	}

	// 서술자 생성
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, dsvHandle);

	return true;
}

void TestGraphics::WaitRender()
{
	const uint64 fenceValue = m_fenceValue[m_currentBackBufferIndex];
	HRESULT hr = m_commendQueue->Signal(m_fence.Get(), fenceValue);
	if (FAILED(hr))
	{
		return;
	}
	m_fenceValue[m_currentBackBufferIndex]++;

	uint64 val = m_fence->GetCompletedValue();
	if (m_fence->GetCompletedValue() < fenceValue)
	{
		auto hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		if (FAILED(hr))
		{
			return;
		}

		// 대기 처리
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE))
		{
			int a = 3;
			return;
		}
	}
}
