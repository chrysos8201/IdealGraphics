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


	// 업로드용 버퍼. 시스템 메모리에 존재한다.
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

	// GPU에서 사용할 버퍼
	class D3D12GPUBuffer : public D3D12Resource
	{
	public:
		D3D12GPUBuffer();
		virtual ~D3D12GPUBuffer();

	protected:
		void CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount);

		// GPU에서 버퍼를 만든다.
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
		// Upload Buffer에 있는 데이터를 GPU Buffer에 복사한다.
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
		// Upload Buffer에 있는 데이터를 GPU Buffer에 복사한다.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		D3D12_INDEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
	};
}