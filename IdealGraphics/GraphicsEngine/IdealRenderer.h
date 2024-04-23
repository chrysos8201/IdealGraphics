#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

#include "GraphicsEngine/D3D12/D3D12Viewport.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12PipelineState.h"
#include "GraphicsEngine/VertexInfo.h"
//#include "GraphicsEngine/Mesh.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"

namespace Ideal
{
	class Mesh;
	class Model;
	class D3D12Texture;
	class D3D12ResourceManager;
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

	// 다시 수정 버전
	void DrawBox();
	void LoadAssets();
	void CreateMeshObject(const std::wstring FileName);
	void BeginRender();
	void EndRender();

	std::shared_ptr<Ideal::D3D12ResourceManager> GetResourceManager();

public:
	ComPtr<ID3D12Device> GetDevice();
	// 일단 cmd list는 하나만 쓴다.
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	//Ideal::D3D12PipelineState
	uint32 GetFrameIndex() const;

	Matrix GetView() { return m_view; }
	Matrix GetProj() { return m_proj; }
	Matrix GetViewProj() { return m_viewProj; }

	// Texture로딩해보는 테스트용 함수
	void LoadBox();
	void CreateBoxTexPipeline();
	void CreateBoxTexture();

	ComPtr<ID3D12Resource> m_tex;
	Ideal::D3D12DescriptorHandle m_texHandle;
	ComPtr<ID3D12RootSignature> m_texRootSignature;

	std::shared_ptr<Ideal::D3D12Texture> m_texture;

	// 2024.04.20 temp 
	std::shared_ptr<Ideal::D3D12Texture> texture;

	// 2024.04.21 Temp : Ideal::DescriptorHeap
	Ideal::D3D12DescriptorHandle AllocateSRV() { return m_idealSrvHeap.Allocate(); }
	uint32 m_srvHeapNum = 256U;
	Ideal::D3D12DescriptorHeap m_idealSrvHeap;

private:
	uint32 m_width;
	uint32 m_height;
	HWND m_hwnd;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	uint32 m_frameIndex;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	// 2024.04.14 : dsv
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	ComPtr<ID3D12Resource> m_depthStencil;

	uint32 m_rtvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_BUFFER_COUNT];
	//ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;



	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

private:
	float m_aspectRatio = 0.f;
	SimpleBoxConstantBuffer m_constantBufferDataSimpleBox;
	const float m_offsetSpeed = 0.02f;
private:

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



private:
	// Temp assimp model import
	std::vector<std::shared_ptr<Ideal::Model>> m_models;


private:
	// BOX
	void BoxTick();

	// 2024.04.11 :
	// VertexBuffer와 IndexBuffer를 묶어줘보겠다.
	Ideal::D3D12VertexBuffer m_idealVertexBuffer;
	Ideal::D3D12IndexBuffer m_idealIndexBuffer;
	Ideal::D3D12ConstantBuffer m_idealConstantBuffer;
	SimpleBoxConstantBuffer* m_simpleBoxConstantBufferDataBegin;

private:
	void CreateCommandList();

	// 2024.04.22 다시 fence를 만든다.
	void CreateFence();
	void Present();
	ComPtr<ID3D12Fence> m_fence;
	uint64 m_fenceValues[FRAME_BUFFER_COUNT];
	HANDLE m_fenceEvent;

	//ComPtr<ID3D12Fence> m_uploadFence;
	//uint64 m_uploadFenceValue;
	//HANDLE m_uploadFenceEvent;

	// Resource Manager
	std::shared_ptr<Ideal::D3D12ResourceManager> m_resourceManager = nullptr;
	
	// Test
	std::shared_ptr<Ideal::D3D12VertexBuffer> m_testVB;
	std::shared_ptr<Ideal::D3D12IndexBuffer> m_testIB;

public:
	void ExecuteCommandList();
};

