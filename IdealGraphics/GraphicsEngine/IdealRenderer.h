#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"

#include "GraphicsEngine/Resources/D3D12Viewport.h"
#include "GraphicsEngine/Resources/D3D12Resource.h"
#include "GraphicsEngine/Resources/D3D12PipelineState.h"
#include "GraphicsEngine/VertexInfo.h"
//#include "GraphicsEngine/Mesh.h"

namespace Ideal
{
	class Mesh;
}

class IdealRenderer : public std::enable_shared_from_this<IdealRenderer>
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };

public:
	IdealRenderer(HWND hwnd, uint32 width, uint32 height);
	virtual ~IdealRenderer();

public:
	void Init();
	void Tick();
	void Render();
	void Release();
	void MoveToNextFrame();

	// 다시 수정 버전
	void PopulateCommandList2();
	void LoadAsset2();

	void ExecuteCommandList();

	void CreateMeshObject(const std::string Path);

	void BeginRender();
	void EndRender();

public:
	ComPtr<ID3D12Device> GetDevice();
	// 일단 cmd list는 하나만 쓴다.
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	//Ideal::D3D12PipelineState
	uint32 GetFrameIndex() const;

	std::shared_ptr<Ideal::D3D12PipelineState> GetPipelineStates();

	Matrix GetView() { return m_view; }
	Matrix GetProj() { return m_proj; }
	Matrix GetViewProj() { return m_viewProj; }
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

	// 2024.04.11 : cbv
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;

	uint32 m_rtvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

private:
	float m_aspectRatio = 0.f;
	SimpleBoxConstantBuffer m_constantBufferDataSimpleBox;
	const float m_offsetSpeed = 0.02f;
private:
	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValues[FRAME_BUFFER_COUNT];
	HANDLE m_fenceEvent;

	void WaitForGPU();

private:
	// 2024.04.14
	// 임시용 wvp
	Matrix m_world;
	Matrix m_viewProj;
	Matrix m_view;
	Matrix m_proj;
private:
	//
	Ideal::D3D12Viewport m_viewport;

	// 2024.04.11 :
	// VertexBuffer와 IndexBuffer를 묶어줘보겠다.
	Ideal::D3D12VertexBuffer m_idealVertexBuffer;
	Ideal::D3D12IndexBuffer m_idealIndexBuffer;
	Ideal::D3D12ConstantBuffer m_idealConstantBuffer;
	SimpleBoxConstantBuffer* m_simpleBoxConstantBufferDataBegin;

	// 2024.04.15 : pso
	std::shared_ptr<Ideal::D3D12PipelineState> m_idealPipelineState;
	ComPtr<ID3D12PipelineState> m_currentPipelineState;

	
private:
	// 2024.04.16 : mesh objects
	std::vector<std::shared_ptr<Ideal::Mesh>> m_objects;
};

