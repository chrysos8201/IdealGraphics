#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"

#include "GraphicsEngine/Resources/D3D12Viewport.h"

class IdealGraphics
{
	enum { FRAME_BUFFER_COUNT  = 2};
public:
	IdealGraphics(HWND hwnd, uint32 width, uint32 height);
	virtual ~IdealGraphics();

public:
	void Init();
	void Tick();
	void Render();
	void PopulateCommandList();
	void MoveToNextFrame();

private:
	uint32 m_width;
	uint32 m_height;
	HWND m_hwnd;
	//CD3DX12_VIEWPORT m_viewport;
	//CD3DX12_RECT m_scissorRect;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	uint32 m_frameIndex;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	uint32 m_rtvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

private:
	float m_aspectRatio = 0.f;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

private:
	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValues[FRAME_BUFFER_COUNT];
	HANDLE m_fenceEvent;

	void WaitForGPU();


private:
	//
	Ideal::D3D12Viewport m_viewport;
};

