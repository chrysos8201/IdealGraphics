#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

// 따로 GPU에 메모리를 업로드 하는 command list를 파서 여기서 사용한다.

namespace Ideal
{
	class D3D12VertexBuffer;
	class D3D12IndexBuffer;
}

namespace Ideal
{
	class D3D12ResourceManager
	{
	public:
		D3D12ResourceManager();
		virtual ~D3D12ResourceManager();

	public:
		void Init(ComPtr<ID3D12Device> Device);

		void Fence();
		void WaitForFenceValue();

		void CreateVertexBufferBox(std::shared_ptr<Ideal::D3D12VertexBuffer>& VertexBuffer);
		void CreateIndexBufferBox(std::shared_ptr<Ideal::D3D12IndexBuffer> IndexBuffer);
		void CreateVertexBufferTest(UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);
	private:
		ComPtr<ID3D12Device> m_device = nullptr;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
		ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
		ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

		ComPtr<ID3D12Fence> m_fence = nullptr;
		uint64 m_fenceValue = 0;
		HANDLE m_fenceEvent = NULL;
	};
}