#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "GraphicsEngine/Resources/D3D12Definitions.h"

namespace Ideal
{
	class D3D12Resource
	{
	public:
		D3D12Resource();
		virtual ~D3D12Resource();

	public:
		ID3D12Resource* GetResource() const;

	protected:
		ComPtr<ID3D12Resource> m_resource = nullptr;
	};


	// ���ε�� ����. �ý��� �޸𸮿� �����Ѵ�.
	class D3D12UploadBuffer : public D3D12Resource
	{
	public:
		D3D12UploadBuffer();
		virtual ~D3D12UploadBuffer();

		void Create(ID3D12Device* Device, uint32 BufferSize);
		void* Map();
		void UnMap();

	private:
		uint32 m_bufferSize;
	};

	// GPU���� ����� ����
	class D3D12GPUBuffer : public D3D12Resource
	{
	public:
		D3D12GPUBuffer();
		virtual ~D3D12GPUBuffer();

	protected:
		void CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount);

		// GPU���� ���۸� �����.
		//void InitializeBuffer();
		uint32 GetBufferSize() const;
		uint32 GetElementCount() const;
		uint32 GetElemnetSize() const;

	protected:
		uint32 m_bufferSize;
		uint32 m_elementSize;
		uint32 m_elementCount;
	};

	// VertexBuffer
	class D3D12VertexBuffer : D3D12GPUBuffer
	{
	public:
		D3D12VertexBuffer();
		virtual ~D3D12VertexBuffer();

	public:
		// Upload Buffer�� �ִ� �����͸� GPU Buffer�� �����Ѵ�.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		
		D3D12_VERTEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	};

	// IndexBuffer
	class D3D12IndexBuffer : D3D12GPUBuffer
	{
	public:
		D3D12IndexBuffer();
		virtual ~D3D12IndexBuffer();

	public:
		// Upload Buffer�� �ִ� �����͸� GPU Buffer�� �����Ѵ�.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		D3D12_INDEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
	};
}