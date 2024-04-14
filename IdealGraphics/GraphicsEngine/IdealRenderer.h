#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"

#include "GraphicsEngine/Resources/D3D12Viewport.h"
#include "GraphicsEngine/Resources/D3D12Resource.h"

class IdealRenderer
{
	enum { FRAME_BUFFER_COUNT = 2 };

	struct TestOffset
	{
		Vector4 offset = Vector4();
		float padding[60];
	};	// 256 byte
	static_assert(sizeof(TestOffset) % 256 == 0);

public:
	IdealRenderer(HWND hwnd, uint32 width, uint32 height);
	virtual ~IdealRenderer();

public:
	void Init();
	void Tick();
	void Render();
	void PopulateCommandList();
	void MoveToNextFrame();

	void LoadAsset();
	// 다시 수정 버전
	void PopulateCommandList2();
	void LoadAsset2();

	void ExecuteCommandList();
public:
	ComPtr<ID3D12Device> GetDevice();
	// 일단 cmd list는 하나만 쓴다.
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

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
	// 2024.04.14 : dsv
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12Resource> m_depthStencil;

	// 2024.04.11 cbv
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;

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

	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	ComPtr<ID3D12Resource> m_constantBuffer;
	uint8* m_cbvDataBegin;				// 상수 버퍼 데이터 시작 주소
	TestOffset m_constantBufferData;	// 테스트용으로 오프셋을 더하는 상수 버퍼용 변수
	//D3D12_CONSTANT_BUFFER_VIEW_DESC m_constantBufferViewDesc;
	const float m_offsetSpeed = 0.02f;
private:
	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValues[FRAME_BUFFER_COUNT];
	HANDLE m_fenceEvent;

	void WaitForGPU();


private:
	//
	Ideal::D3D12Viewport m_viewport;

	// 2024.04.11 :
	// VertexBuffer와 IndexBuffer를 묶어줘보겠다.
	Ideal::D3D12VertexBuffer m_idealVertexBuffer;
	Ideal::D3D12IndexBuffer m_idealIndexBuffer;
	Ideal::D3D12ConstantBuffer m_idealConstantBuffer;
	TestOffset* m_testOffsetConstantBufferDataBegin;
};

