#include "RHIMemoryPool.h"
#include "d3d12.h"
#include "d3dx12_core.h"
#include "D3D12\D3D12Common.h"

template <typename T>
constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

uint32 Ideal::RHIMemoryPool::GetAlignedSize(uint32 InSizeInBytes, uint32 InPoolAlignment, uint32 InAllocationAlignment)
{
	Check(InAllocationAlignment <= InPoolAlignment);

	uint32 Size = ((InPoolAlignment % InAllocationAlignment) != 0) ? InSizeInBytes + InAllocationAlignment : InSizeInBytes;
	return AlignArbitrary(Size, InPoolAlignment);	// ����
}

//using namespace Ideal;

Ideal::RHIMemoryPool::RHIMemoryPool(uint64 InPoolSize, uint32 InPoolAlignment)
	: PoolSize(InPoolSize)
	, PoolAlignment(InPoolAlignment)
{

}

void Ideal::RHIMemoryPool::Init()
{

}


bool Ideal::RHIMemoryPool::TryAllocate(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& AllocationData)
{
	// TODO : ���� ä������ // 2025.06.18 20:14
	return false;
}

/// <summary>
///  D3D12MemoryPool
/// </summary>
/// <param name="Device"></param>

Ideal::D3D12MemoryPool::D3D12MemoryPool(
	ComPtr<ID3D12Device> InDevice
	, uint32 InPoolIndex
	, uint64 InPoolSize
	, uint32 InPoolAlignment
	, EResourceAllocationStrategy InAllocationStrategy
	, D3D12ResourceInitConfig InInitConfig
	) : RHIMemoryPool(InPoolSize, InPoolAlignment)
	, D3D12DeviceChild(Device)
	, PoolIndex(InPoolIndex)
	, AllocationStrategy(InAllocationStrategy)
	, InitConfig(InInitConfig)
{

}

void Ideal::D3D12MemoryPool::Init()
{
	if (PoolSize == 0)
	{
		return;
	}

	// ID3D12Heap�� ����� ���
	if (AllocationStrategy == EResourceAllocationStrategy::PlacedResource)
	{
		// GPU Resource �޸� ������ 4KB �̰ų� 64KB�̾�� �Ѵ�
		Check(PoolAlignment == D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT || PoolAlignment == D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		
		CD3DX12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InitConfig.HeapType);
		
		D3D12_HEAP_DESC Desc = {};
		Desc.SizeInBytes = PoolSize;
		Desc.Properties = HeapProps;
		Desc.Alignment = 0;
		Desc.Flags = InitConfig.HeapFlags;

		// Create heap
		Device->CreateHeap(&Desc, IID_PPV_ARGS(BackingHeap.GetAddressOf()));
	}
	// ID3D12Resource�� ����� ���
	else
	{
		CD3DX12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InitConfig.HeapType);
		CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(PoolSize);
		Device->CreateCommittedResource(
			&HeapProps,
			InitConfig.HeapFlags,
			&ResourceDesc,
			InitConfig.InitialResourceState,
			nullptr,
			IID_PPV_ARGS(BackingResource.GetAddressOf())
		);
	}

	RHIMemoryPool::Init();
}