#pragma once
#include "Core//Core.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "RenderTest/VertexInfo.h"

class GraphicsEngine
{
	enum { FRAME_BUFFER_COUNT = 2 };

public:
	GraphicsEngine(HWND hwnd, uint32 width, uint32 height);
	virtual ~GraphicsEngine();

public:
	void Init();
	void BeginRender();
	void EndRender();

	void Tick();
	void Render();
public:
	void LoadPipeline();
	void LoadAsset();

	void PopulateCommandList();
private:
	void WaitForPreviousFrame();

private:
	// 먼저 Viewport 세팅
	HWND m_hwnd;
	uint32 m_width;
	uint32 m_height;
	float m_aspectRatio;
private:
	uint32 m_frameIndex;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	uint32 m_rtvDescriptorSize;

	ComPtr<ID3D12Resource> m_renderTargets[2];

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValue = 0;
	HANDLE m_fenceEvent;

private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	
	ComPtr<ID3D12Resource> m_constantBuffer;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	uint8* m_cbvDataBegin;
	Transform m_transform;

	float m_rotateY = 0.f;
};

