#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"

class EngineTest2
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };	// 더블 버퍼링

public:
	EngineTest2(HWND hwnd, uint32 width, uint32 height);
	virtual ~EngineTest2();

public:
	void Init();
	void Tick();
	void Render();

	void BeginRender();
	void EndRender();

public:
	ID3D12Device6* GetDevice();
	ID3D12GraphicsCommandList* GetCommandList();
	uint32 CurrentBackBufferIndex();

private:
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateCommandList();
	bool CreateFence();
	void CreateViewport();
	void CreateScissorRect();

private:
	HWND m_hwnd;
	uint32 m_frameBufferWidth = 0;
	uint32 m_frameBufferHeight = 0;
	uint32 m_currentBackBufferIndex = 0;

	ComPtr<ID3D12Device6> m_device = nullptr;
	ComPtr<ID3D12CommandQueue> m_queue = nullptr;
	ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> m_allocator[FRAME_BUFFER_COUNT] = { nullptr };
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;
	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> m_fence = nullptr;
	uint64 m_fenceValue[FRAME_BUFFER_COUNT];
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

private:
	bool CreateRenderTarget();
	bool CreateDepthStencil();

	uint32 m_rtvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap = nullptr;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT] = { nullptr };

	uint32 m_DSVDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_DSVHeap = nullptr;
	ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;

private:
	ID3D12Resource* m_currentRenderTarget = nullptr;
	void WaitRender();
};

extern EngineTest2* g_Engine;

