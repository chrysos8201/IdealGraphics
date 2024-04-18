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


	// 업로드용 임시 버퍼; 업로드 힙에 잡힌다. cpu write gpu read
	class D3D12UploadBuffer : public D3D12Resource
	{
	public:
		D3D12UploadBuffer();
		virtual ~D3D12UploadBuffer();

		// 업로드 버퍼를 만든다. 
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

	public:
		// GPU에서 사용할 버퍼를 만든다.
		// 만들 때 리소스 스테이트를 COPY_DEST로 초기화 해준다.
		void CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount);

		// GPU에서 버퍼를 만든다.
		//void InitializeBuffer();
		uint32 GetBufferSize() const;
		uint32 GetElementCount() const;
		uint32 GetElemnetSize() const;

	protected:
		void SetName(const LPCTSTR& name);

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
		// Upload Buffer에 있는 데이터를 GPU Buffer에 복사한다.
		// 내부에서 리소스 베리어를 걸어주고 Buffer View를 만든다.
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
		// Upload Buffer에 있는 데이터를 GPU Buffer에 복사한다.
		// 내부에서 리소스 베리어를 걸어주고 Buffer View를 만든다.
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		D3D12_INDEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
	};

	// ConstantBuffer
	// 업로드 힙에 잡는다. cpu write / gpu read
	// 매 프레임 수정할 메모리이기 땜문에 일일히 GPU에 넣어서 fence/wait을 걸어주지 않는다.
	// 두 프레임의 메모리를 잡아주지만 매 프레임마다 wait를 걸기 때문에 의미가 없어보임.
	class D3D12ConstantBuffer : public D3D12Resource
	{
	public:
		D3D12ConstantBuffer();
		virtual ~D3D12ConstantBuffer();

	public:
		void Create(ID3D12Device* Device, uint32 BufferSize, uint32 FrameCount);
		// 현재 프레임 인덱스에 해당하는 메모리의 주소를 가져온다.
		void* GetMappedMemory(uint32 FrameIndex);

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(uint32 FrameIndex);
	private:
		uint32 Align(uint32 location, uint32 align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	private:
		// 총 버퍼의 사이즈 : 버퍼 * 프레임 개수
		uint32 m_bufferSize;

		// 한 프레임의 버퍼 사이즈
		uint32 m_perFrameBufferSize;

		// 버퍼를 할당받은 영역의 첫 주소를 가리킨다.
		void* m_mappedConstantBuffer;

		bool m_isMapped;
	};
}