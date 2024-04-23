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
	void LoadAssets();
	void CreateMeshObject(const std::wstring FileName);
	void BeginRender();
	void EndRender();

	std::shared_ptr<Ideal::D3D12ResourceManager> GetResourceManager();

	// Command
	void CreateCommandList();

	// Fence
	void CreateGraphicsFence();
	uint64 GraphicsFence();
	void WaitForGraphicsFenceValue();

	void GraphicsPresent();

public:
	ComPtr<ID3D12Device> GetDevice();
	// 일단 cmd list는 하나만 쓴다.
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	//Ideal::D3D12PipelineState
	uint32 GetFrameIndex() const;

	Matrix GetView() { return m_view; }
	Matrix GetProj() { return m_proj; }
	Matrix GetViewProj() { return m_viewProj; }

	// 2024.04.21 Temp : Ideal::DescriptorHeap
	Ideal::D3D12DescriptorHandle AllocateSRV() { return m_idealSrvHeap.Allocate(); }
	uint32 m_srvHeapNum = 256U;
	Ideal::D3D12DescriptorHeap m_idealSrvHeap;

private:
	uint32 m_width;
	uint32 m_height;
	HWND m_hwnd;

	// Device
	ComPtr<ID3D12Device> m_device = nullptr;
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
	ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
	uint32 m_frameIndex;

	// RTV
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
	uint32 m_rtvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT];

	// DSV
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
	ComPtr<ID3D12Resource> m_depthStencil = nullptr;

	// Command
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	// Fence
	ComPtr<ID3D12Fence> m_graphicsFence = nullptr;
	uint64 m_graphicsFenceValue;
	HANDLE m_graphicsFenceEvent;

private:
	float m_aspectRatio = 0.f;

private:
	const float m_offsetSpeed = 0.02f;

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
	// Resource Manager
	std::shared_ptr<Ideal::D3D12ResourceManager> m_resourceManager = nullptr;

private:
	//-----------BOX----------//
	void LoadBox();
	void DrawBox();
	void BoxTick();

	// box init
	void CreateBoxTexPipeline();
	void CreateBoxTexture();

	// 2024.04.11 :
	// VertexBuffer와 IndexBuffer를 묶어줘보겠다.
	std::shared_ptr<Ideal::D3D12VertexBuffer> m_boxVB = nullptr;
	std::shared_ptr<Ideal::D3D12IndexBuffer> m_boxIB = nullptr;
	Ideal::D3D12ConstantBuffer m_boxCB;
	std::shared_ptr<Ideal::D3D12Texture> m_boxTexture = nullptr;
	SimpleBoxConstantBuffer* m_simpleBoxConstantBufferDataBegin = nullptr;

	ComPtr<ID3D12RootSignature> m_boxRootSignature;
	ComPtr<ID3D12PipelineState> m_boxPipeline;

	//------------------------//
};