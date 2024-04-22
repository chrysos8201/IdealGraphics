#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

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


	// ���ε�� �ӽ� ����; ���ε� ���� ������. cpu write gpu read
	class D3D12UploadBuffer : public D3D12Resource
	{
	public:
		D3D12UploadBuffer();
		virtual ~D3D12UploadBuffer();

		// ���ε� ���۸� �����. 
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

		void SetName(const LPCTSTR& name);

	public:
		// GPU���� ����� ���۸� �����.
		// ���� �� ���ҽ� ������Ʈ�� COPY_DEST�� �ʱ�ȭ ���ش�.
		void CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount);

		// GPU���� ���۸� �����.
		//void InitializeBuffer();
		uint32 GetBufferSize() const;
		uint32 GetElementCount() const;
		uint32 GetElemnetSize() const;

	protected:
		std::wstring m_name;
		uint32 m_bufferSize;
		uint32 m_elementSize;
		uint32 m_elementCount;
	};

	// VertexBuffer
	class D3D12VertexBuffer : public D3D12GPUBuffer
	{
	public:
		D3D12VertexBuffer();
		virtual ~D3D12VertexBuffer();

	public:
		// Upload Buffer�� �ִ� �����͸� GPU Buffer�� �����Ѵ�.
		// ���ο��� ���ҽ� ����� �ɾ��ְ� Buffer View�� �����.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		
		D3D12_VERTEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	};

	// IndexBuffer
	class D3D12IndexBuffer : public D3D12GPUBuffer
	{
	public:
		D3D12IndexBuffer();
		virtual ~D3D12IndexBuffer();

	public:
		// Upload Buffer�� �ִ� �����͸� GPU Buffer�� �����Ѵ�.
		// ���ο��� ���ҽ� ����� �ɾ��ְ� Buffer View�� �����.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		D3D12_INDEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
	};

	// ConstantBuffer
	// ���ε� ���� ��´�. cpu write / gpu read
	// �� ������ ������ �޸��̱� ������ ������ GPU�� �־ fence/wait�� �ɾ����� �ʴ´�.
	// �� �������� �޸𸮸� ��������� �� �����Ӹ��� wait�� �ɱ� ������ �ǹ̰� �����.
	class D3D12ConstantBuffer : public D3D12Resource
	{
	public:
		D3D12ConstantBuffer();
		virtual ~D3D12ConstantBuffer();

	public:
		void Create(ID3D12Device* Device, uint32 BufferSize, uint32 FrameCount);
		// ���� ������ �ε����� �ش��ϴ� �޸��� �ּҸ� �����´�.
		void* GetMappedMemory(uint32 FrameIndex);

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(uint32 FrameIndex);
	private:
		uint32 Align(uint32 location, uint32 align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	private:
		// �� ������ ������ : ���� * ������ ����
		uint32 m_bufferSize;

		// �� �������� ���� ������
		uint32 m_perFrameBufferSize;

		// ���۸� �Ҵ���� ������ ù �ּҸ� ����Ų��.
		void* m_mappedConstantBuffer;

		bool m_isMapped;
	};
}