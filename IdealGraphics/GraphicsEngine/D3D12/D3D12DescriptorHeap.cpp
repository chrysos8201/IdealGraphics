#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorManager.h"

using namespace Ideal;


//------------------Descriptor Handle-------------------//

D3D12DescriptorHandle::D3D12DescriptorHandle()
	: m_cpuHandle(),
	m_gpuHandle()
{
	m_cpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}

D3D12DescriptorHandle::D3D12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle, std::shared_ptr<Ideal::D3D12DescriptorHeapBase> OwnerHeap, int32 AllocatedIndex, int32 FixedHeapIndex/* =-1*/)
	: m_cpuHandle(CpuHandle),
	m_gpuHandle(GpuHandle),
	m_ownerHeap(OwnerHeap),
	m_allocatedIndex(AllocatedIndex),
	m_fixedHeapIndex(FixedHeapIndex)
{
	//m_ownerHeap = OwnerHeap;
}

D3D12DescriptorHandle::D3D12DescriptorHandle(const D3D12DescriptorHandle& Other)
{
	m_cpuHandle = Other.m_cpuHandle;
	m_gpuHandle = Other.m_gpuHandle;
	m_ownerHeap = Other.m_ownerHeap;
	m_allocatedIndex = Other.m_allocatedIndex;
	m_fixedHeapIndex = Other.m_fixedHeapIndex;
}

D3D12DescriptorHandle::D3D12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
	: m_cpuHandle(CpuHandle),
	m_gpuHandle(GpuHandle),
	m_ownerHeap(),
	m_allocatedIndex(-1),
	m_fixedHeapIndex(-1)
{

}

D3D12DescriptorHandle::~D3D12DescriptorHandle()
{
	/*if (!m_ownerHeap.expired())
	{
		m_ownerHeap.lock()->Free(m_allocatedIndex);
		m_ownerHeap.reset();
	}*/
}

void D3D12DescriptorHandle::Free()
{
	if (!m_ownerHeap.expired())
	{
		Ideal::EDescriptorType type = m_ownerHeap.lock()->GetDescriptorType();

		switch (type)
		{
			case Ideal::EDescriptorType::None:
			{

			}
				break;
			case Ideal::EDescriptorType::StaticHeap:
			{

			}
				break;
			case Ideal::EDescriptorType::DynamicHeap:
			{
				std::static_pointer_cast<Ideal::D3D12DynamicDescriptorHeap>(m_ownerHeap.lock())->Free(m_allocatedIndex);
			}
				break;
			case Ideal::EDescriptorType::Static_DynamicHeap:
			{
				std::static_pointer_cast<Ideal::D3D12DescriptorManager>(m_ownerHeap.lock())->Free(m_fixedHeapIndex, m_allocatedIndex);
			}
				break;
			default:
				break;
		}
		type = Ideal::EDescriptorType::None;
		/*m_ownerHeap.lock()->Free(m_allocatedIndex);
		m_ownerHeap.reset();*/
	}
}

//------------------Descriptor Heaap-------------------//

D3D12DescriptorHeap::D3D12DescriptorHeap()
	: m_numFreeDescriptors(0),
	m_descriptorSize(0),
	m_maxCount(0)
{

}

D3D12DescriptorHeap::~D3D12DescriptorHeap()
{

}

void D3D12DescriptorHeap::Create(ID3D12Device* Device, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, D3D12_DESCRIPTOR_HEAP_FLAGS Flags, uint32 MaxCount)
{
	m_maxCount = MaxCount;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MaxCount;
	heapDesc.Type = HeapType;
	heapDesc.Flags = Flags;
	Check(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

	m_descriptorSize = Device->GetDescriptorHandleIncrementSize(heapDesc.Type);
	m_numFreeDescriptors = heapDesc.NumDescriptors;

	// 첫 위치로 Handle을 만든다.
	m_freeHandle = Ideal::D3D12DescriptorHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_descriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);

	m_headHandle = m_freeHandle;
}

Ideal::D3D12DescriptorHandle D3D12DescriptorHeap::Allocate(uint32 Count /* = 1*/)
{
	if (Count == 0)
	{
		MessageBox(NULL, L"Cant' Allocate Descriptor", L"오류", MB_OK);
		assert(false);
		return Ideal::D3D12DescriptorHandle();
	}
	// 할당할 수 있는 개수를 넘어섰을 경우
	if (m_numFreeDescriptors < Count)
	{
		MessageBox(NULL, L"Cant' Allocate Descriptor", L"오류", MB_OK);
		assert(false);
		return Ideal::D3D12DescriptorHandle();
	}

	// return 값은 FreeHandle의 주소를 이동시키기 전 , 즉 현재 비어있는 descriptor heap의 주소를 가지고 만든다.
	Ideal::D3D12DescriptorHandle ret = m_freeHandle;
	m_freeHandle += Count * m_descriptorSize;

	m_numFreeDescriptors -= Count;

	return ret;
}

void D3D12DescriptorHeap::Reset()
{
	m_count = 0;
	m_numFreeDescriptors = m_maxCount;
	m_freeHandle = m_headHandle;

}

//--------------Dynamic--------------//

D3D12DynamicDescriptorHeap::D3D12DynamicDescriptorHeap()
	: m_descriptorSize(0),
	m_maxCount(0),
	m_isShaderVisible(true)
{

}

D3D12DynamicDescriptorHeap::~D3D12DynamicDescriptorHeap()
{
	int a = 3;
}

void D3D12DynamicDescriptorHeap::Create(ID3D12Device* Device, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, D3D12_DESCRIPTOR_HEAP_FLAGS Flags, uint32 MaxCount)
{
	if (Flags == D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
		m_isShaderVisible = false;

	m_maxCount = MaxCount;
	m_indexCreator.Init(MaxCount);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MaxCount;
	heapDesc.Type = HeapType;
	heapDesc.Flags = Flags;
	Check(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

	m_descriptorSize = Device->GetDescriptorHandleIncrementSize(heapDesc.Type);
}

Ideal::D3D12DescriptorHandle D3D12DynamicDescriptorHeap::Allocate(uint32 Count /*= 1*/)
{
	if (Count == 0)
	{
		MessageBox(NULL, L"Cant' Allocate Descriptor", L"오류", MB_OK);
		assert(false);
		return Ideal::D3D12DescriptorHandle();
	}

	int32 index = m_indexCreator.Allocate();

	D3D12_GPU_DESCRIPTOR_HANDLE gpuAddress;
	gpuAddress.ptr = -1;
	if (m_isShaderVisible)
	{
		gpuAddress = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}

	Ideal::D3D12DescriptorHandle ret = Ideal::D3D12DescriptorHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		//m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
		gpuAddress,
		shared_from_this(),
		index
	);
	ret += index * m_descriptorSize;
	return ret;
}

void D3D12DynamicDescriptorHeap::Free(uint32 Index)
{
	m_indexCreator.Free(Index);
}
