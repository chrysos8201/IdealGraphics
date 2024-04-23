#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/VertexInfo.h"

#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"

// 따로 GPU에 메모리를 업로드 하는 command list를 파서 여기서 사용한다.

namespace Ideal
{
	class D3D12VertexBuffer;
	class D3D12IndexBuffer;
	class D3D12Texture;
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

		ComPtr<ID3D12DescriptorHeap> GetSRVHeap() { return m_srvHeap.GetDescriptorHeap(); }

		void CreateVertexBufferBox(std::shared_ptr<Ideal::D3D12VertexBuffer>& VertexBuffer);
		void CreateIndexBufferBox(std::shared_ptr<Ideal::D3D12IndexBuffer> IndexBuffer);


		template <typename TVertexType>
		void CreateVertexBuffer(std::shared_ptr<Ideal::D3D12VertexBuffer> OutVertexBuffer,
			std::vector<TVertexType>& Vertices
		)
		{
			m_commandAllocator->Reset();
			m_commandList->Reset(m_commandAllocator.Get(), nullptr);

			const uint32 elementSize = sizeof(TVertexType);
			const uint32 elementCount = Vertices.size();
			const uint32 bufferSize = elementSize * elementCount;

			Ideal::D3D12UploadBuffer uploadBuffer;
			uploadBuffer.Create(m_device.Get(), bufferSize);
			{
				void* mappedData = uploadBuffer.Map();
				memcpy(mappedData, Vertices.data(), bufferSize);
				uploadBuffer.UnMap();
			}
			OutVertexBuffer->Create(m_device.Get(),
				m_commandList.Get(),
				elementSize,
				elementCount,
				uploadBuffer
			);

			//---------Execute---------//
			m_commandList->Close();
			ID3D12CommandList* commandLists[] = { m_commandList.Get() };
			m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

			Fence();
			WaitForFenceValue();
		}

		void CreateIndexBuffer(std::shared_ptr<Ideal::D3D12IndexBuffer> OutIndexBuffer,
			std::vector<uint32>& Indices
		);

		void CreateTexture(std::shared_ptr<Ideal::D3D12Texture> OutTexture, const std::wstring& Path);

	private:
		ComPtr<ID3D12Device> m_device = nullptr;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
		ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
		ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

		ComPtr<ID3D12Fence> m_fence = nullptr;
		uint64 m_fenceValue = 0;
		HANDLE m_fenceEvent = NULL;

	private:
		// Descriptor heaps
		Ideal::D3D12DescriptorHeap m_srvHeap;
		const uint32 m_srvHeapCount = 256U;
	};
}