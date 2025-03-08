#pragma once
#include <d3d12.h>
// 2025.03.06
// Descriptor 구조 변경..
#include "GraphicsEngine/D3D12/D3D12Descriptors.h"

struct ID3D12Resource;
struct ID3D12Device;

namespace Ideal
{
	class D3D12UnorderedAccessView;
	class D3D12UAVBuffer;
	class D3D12DescriptorHandle;
	class D3D12DescriptorHandle2;
}
namespace Ideal
{
	class D3D12Resource
	{
	public:
		D3D12Resource();
		virtual ~D3D12Resource();
		void Release();

	public:
		ID3D12Resource* GetResource() const;
		ComPtr<ID3D12Resource> GetResourceComPtr();

	public:
		// 2025.03.06
		// View들을 다 여기로 옮겨버리겠다.
		// Descriptor Handle

	public:
		// 2025.03.06
		// Descriptor 구조를 변경하면서 핸들도 새로운 것으로 바꾸겠다.
		void EmplaceSRV2(const D3D12DescriptorHandle2& InSRVHandle);
		void EmplaceRTV2(const D3D12DescriptorHandle2& InRTVHandle);
		void EmplaceUAV2(const D3D12DescriptorHandle2& InUAVHandle);
		void EmplaceDSV2(const D3D12DescriptorHandle2& InDSVHandle);

		D3D12DescriptorHandle2 GetSRV2() const;
		D3D12DescriptorHandle2 GetRTV2() const;
		D3D12DescriptorHandle2 GetUAV2() const;
		D3D12DescriptorHandle2 GetDSV2() const;

		void FreeHandle();

	protected:
		D3D12DescriptorHandle2 m_srvHandle2;
		D3D12DescriptorHandle2 m_rtvHandle2;
		D3D12DescriptorHandle2 m_uavHandle2;
		D3D12DescriptorHandle2 m_dsvHandle2;

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
		void* MapCpuWriteOnly();
		void UnMap();

	private:
		uint32 m_bufferSize;
	};
	
	//template <class T>
	//class D3D12StructuredBuffer : public D3D12UploadBuffer
	//{
	//public:
	//	void Create(ID3D12Device* Device, uint32 NumElements, LPCWSTR ResourceName = nulltpr)
	//	{
	//		m_staging.resize(NumElements);
	//		uint32 bufferSize = NumElements * sizeof(T);
	//		D3D12UploadBuffer::Create(Device, bufferSize);
	//		m_mappedBuffers = reinterpret_cast<T*>(D3D12UploadBuffer::Map());
	//	}
	//
	//	// 현재 까지 업로드 버퍼에 올린 것을 GPU에 올린다. //
	//	void CopyStagingToGPU()
	//	{
	//		memcpy(m_mappedBuffers, &m_staging[0], m_staging.size() * sizeof(T));
	//	}
	//
	//	T& operator[](uint32 ElementIndex)
	//	{
	//		if (ElementIndex > m_staging.size() - 1)
	//		{
	//			__debugbreak();
	//			return;
	//		}
	//		return m_staging[ElementIndex];
	//	}
	//
	//	const T& operator[](uint32 ElementIndex)
	//	{
	//		if (ElementIndex > m_staging.size() - 1)
	//		{
	//			__debugbreak();
	//			return;
	//		}
	//		return m_staging[ElementIndex];
	//	}
	//
	//private:
	//	std::vector<T> m_staging;
	//	T* m_mappedBuffers;
	//};

	// GPU에서 사용할 버퍼
	class D3D12GPUBuffer : public D3D12Resource
	{
	public:
		D3D12GPUBuffer();
		virtual ~D3D12GPUBuffer();

		void SetName(const LPCTSTR& name);

	public:
		// GPU에서 사용할 버퍼를 만든다.
		// 만들 때 리소스 스테이트를 COPY_DEST로 초기화 해준다.
		void CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount, D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE);

		// GPU에서 버퍼를 만든다.
		//void InitializeBuffer();
		uint32 GetBufferSize() const;
		uint32 GetElementCount() const;
		uint32 GetElementSize() const;

	protected:
		std::wstring m_name;
		uint32 m_bufferSize;
		uint32 m_elementSize;
		uint32 m_elementCount;
	};

	class D3D12StructuredBuffer : public D3D12GPUBuffer
	{
	public:
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer);
		void Free();

		//void EmplaceSRV(Ideal::D3D12DescriptorHandle SRVHandle);
		//void EmplaceUAV(Ideal::D3D12DescriptorHandle UAVHandle);

		//Ideal::D3D12DescriptorHandle GetSRV();
		//Ideal::D3D12DescriptorHandle GetUAV();

		void TransitionToSRV(ID3D12GraphicsCommandList* CmdList);
		void TransitionToUAV(ID3D12GraphicsCommandList* CmdList);

	private:
		//Ideal::D3D12DescriptorHandle m_srvHandle;
		//Ideal::D3D12DescriptorHandle m_uavHandle;

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
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer);
		void CreateAndCopyResource(ComPtr<ID3D12Device> Device, uint32 ElementSize, uint32 ElementCount, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState);

		D3D12_VERTEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
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
		void Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, std::shared_ptr<D3D12UploadBuffer> UploadBuffer);
		D3D12_INDEX_BUFFER_VIEW GetView() const;

	private:
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
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

	class D3D12UAVBuffer : public D3D12Resource
	{
		// dㅏ// 이거 만들면 안됐는데 그냥 Resource에 나중에 UAV를 추가하든 뭔가 해야한다.
	public:
		D3D12UAVBuffer();
		virtual ~D3D12UAVBuffer();

	public:
		void Create(ID3D12Device* Device, uint32 BufferSize, D3D12_RESOURCE_STATES InitialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* Name = nullptr);
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();

		void SetUAV(std::shared_ptr<Ideal::D3D12UnorderedAccessView> UAV);
		std::shared_ptr<Ideal::D3D12UnorderedAccessView> GetUAV();
	private:
		std::shared_ptr<Ideal::D3D12UnorderedAccessView> m_uav;
	};
}