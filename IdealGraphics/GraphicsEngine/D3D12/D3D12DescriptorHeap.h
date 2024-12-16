#pragma once
#include "Core/Types.h"
#include <memory>
#include <d3d12.h>
#include <d3dx12.h>
#include "Misc/Utils/IndexCreator.h"

namespace Ideal
{
	class D3D12Renderer;
	class D3D12DescriptorHeap;
	class D3D12DynamicDescriptorHeap;
	class D3D12DescriptorHeapBase;
}

namespace Ideal
{
	enum class EDescriptorType
	{
		None,
		// descriptor heap
		StaticHeap,
		// dynamic heap
		DynamicHeap,
		// manager
		Static_DynamicHeap,
	};

	//--------------------------Descriptor Handle---------------------------//

	class D3D12DescriptorHandle
	{
		friend class Ideal::D3D12DescriptorHeap;
		friend class Ideal::D3D12DynamicDescriptorHeap;

	public:
		D3D12DescriptorHandle();
		virtual ~D3D12DescriptorHandle();

		D3D12DescriptorHandle(const D3D12DescriptorHandle& Other);
		D3D12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle, std::shared_ptr<Ideal::D3D12DescriptorHeapBase> OwnerHeap, int32 AllocatedIndex, int32 FixedHeapIndex = -1);
		D3D12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle);
	
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_gpuHandle; }

		void Free();

	public:
		// operators
		void operator=(const D3D12DescriptorHandle& Other)
		{
			//if (m_ownerHeap.expired() == false)
			//	Free();

			m_cpuHandle = Other.m_cpuHandle;
			m_gpuHandle = Other.m_gpuHandle;
			m_ownerHeap = Other.m_ownerHeap;
			m_allocatedIndex = Other.m_allocatedIndex;
			m_fixedHeapIndex = Other.m_fixedHeapIndex;
		}
	
		void operator+=(uint32 IncrementSize)
		{
			m_cpuHandle.ptr += IncrementSize;
			m_gpuHandle.ptr += IncrementSize;
		}

	private:
		//CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

		// allocated index
		//std::weak_ptr<Ideal::D3D12DynamicDescriptorHeap> m_ownerHeap;
		std::weak_ptr<Ideal::D3D12DescriptorHeapBase> m_ownerHeap;
		int32 m_allocatedIndex = -1;

		// DescriptorManger일 경우 
		int32 m_fixedHeapIndex = -1;
	};


	////////////////////////////////////////////////////////////////////////
	//--------------------------Descriptor Heap---------------------------//
	////////////////////////////////////////////////////////////////////////


	class D3D12DescriptorHeapBase
	{
	public:
		virtual EDescriptorType GetDescriptorType() { return EDescriptorType::None; }

	protected:
	};

	class D3D12DescriptorHeap : public D3D12DescriptorHeapBase, public std::enable_shared_from_this<Ideal::D3D12DescriptorHeap>
	{
	public:
		D3D12DescriptorHeap();
		virtual ~D3D12DescriptorHeap();

		virtual EDescriptorType GetDescriptorType() override { return EDescriptorType::StaticHeap; }

	public:
		void Create(ID3D12Device* Device, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, D3D12_DESCRIPTOR_HEAP_FLAGS Flags, uint32 MaxCount);
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descriptorHeap; }

		// 메모리를 할당할 주소를 받아오고 Count만큼 Handle을 이동시킨다.
		Ideal::D3D12DescriptorHandle Allocate(uint32 Count = 1);
		void Reset();

	public:
		void SetName(const std::wstring& Name) { m_descriptorHeap->SetName(Name.c_str()); }

	private:
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

		// 앞으로 descriptor를 할당할 수 있는 개수
		uint32 m_numFreeDescriptors;
		uint32 m_descriptorSize;
		uint32 m_maxCount;
		uint32 m_count = 0;
		// 현재 할당할 수 있는 주소를 가르키고 있는 핸들
		Ideal::D3D12DescriptorHandle m_freeHandle;
		// 첫 위치 핸들
		Ideal::D3D12DescriptorHandle m_headHandle;
	};

	//--------------Dynamic--------------//

	class D3D12DynamicDescriptorHeap : public D3D12DescriptorHeapBase ,public std::enable_shared_from_this<Ideal::D3D12DynamicDescriptorHeap>
	{
	public:
		D3D12DynamicDescriptorHeap();
		virtual ~D3D12DynamicDescriptorHeap();

		virtual EDescriptorType GetDescriptorType() override { return EDescriptorType::DynamicHeap; }

	public:
		void Create(ID3D12Device* Device, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, D3D12_DESCRIPTOR_HEAP_FLAGS Flags, uint32 MaxCount);
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descriptorHeap; }

		// 메모리를 할당할 주소를 받아오고 Count만큼 Handle을 이동시킨다.
		Ideal::D3D12DescriptorHandle Allocate(uint32 Count = 1);
		void Free(uint32 Index);

		D3D12_CPU_DESCRIPTOR_HANDLE GetFirstCPUDescriptorHandle() { return m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(); }
		uint32 GetDescriptorSize() { return m_descriptorSize * m_maxCount; }
		uint32 GetDescriptorMaxCount() { return m_maxCount; }
	public:
		void SetName(const std::wstring& Name) { m_descriptorHeap->SetName(Name.c_str()); }

	private:
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

		// 앞으로 descriptor를 할당할 수 있는 개수
		uint32 m_descriptorSize;
		uint32 m_maxCount;
		uint32 m_count = 0;

		bool m_isShaderVisible;
		// Index
		IndexCreator m_indexCreator;
	};
}