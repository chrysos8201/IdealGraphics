#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"
#include <DirectXColors.h>
#include "VertexInfo.h"
#include "ThirdParty/Common/UploadBuffer.h"
#include "ThirdParty/Common/d3dUtil.h"
struct ObjectConstants
{
	Matrix WorldViewProj = Matrix::Identity;
};


class EngineTest
{
public:
	EngineTest(HWND hwnd);
	virtual ~EngineTest();

public:
	void Init();
	void Tick();
	void Render();
	void End();

private:
	void InitD3D();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRTVAndDSVDescriptorHeaps();
	void CreateRTV();
	void CreateDSV();
	void SetViewport();
	void SetScissorRect();

	void Draw();
	
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	ID3D12Resource* CurrentBackBuffer() const;

	void FlushCommendQueue();


private:

	ComPtr<ID3D12Device> m_d3dDevice;
	ComPtr<IDXGIFactory4> m_dxgiFactory;
	ComPtr<IDXGISwapChain> m_swapChain;

	ComPtr<ID3D12Fence> m_fence;
	uint32 m_currentFence = 0;

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_directCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	static const int SwapChainBufferCount = 2;
	int32 m_currentBackBuffer = 0;

	ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;
	
	uint32 m_RTVDescriptorSize = 0;
	uint32 m_DSVDescriptorSize = 0;
	uint32 m_CBVSRVDescriptorSize = 0;

private:
	HWND m_hMainWnd = nullptr;	// 주 창 핸들
	bool m_FullScreenState = false;

	// msaa
	bool m_4xMsaaState = false;
	uint32 m_4xMsaaQuality = 0;

private:
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int32 m_clientWidth = 1280;
	int32 m_clientHeight = 960;



	//

private:
	void BuildBoxGeometry();
	void CreateVertexBuffer();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> m_objectCB = nullptr;
	std::unique_ptr<MeshGeometry> m_boxGeometry;
	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	ComPtr<ID3D12PipelineState> m_PSO = nullptr;

};