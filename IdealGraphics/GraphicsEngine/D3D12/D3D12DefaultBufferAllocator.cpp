#include "D3D12DefaultBufferAllocator.h"
#include "D3D12Definitions.h"
#include "RHI\RHIPoolAllocator.h"

Ideal::D3D12DefaultBufferAllocator::D3D12DefaultBufferAllocator(ComPtr<ID3D12Device> InDevice)
	: D3D12DeviceChild(InDevice)
{

}


void Ideal::D3D12DefaultBufferAllocator::AllocateDefaultResource(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, const D3D12_RESOURCE_DESC& InResourceDesc, D3D12_RESOURCE_STATES InInitialResourceState, ED3D12ResourceStateMode InResourceStateMode, D3D12_RESOURCE_STATES InCreateState, D3D12ResourceLocation& ResourceLocation, uint32 Alignment)
{
	// TODO : Resource Location :: Clear

	D3D12PoolAllocator* BufferPool = nullptr;
	for (D3D12PoolAllocator* Pool : DefaultBufferPools)
	{
		// �ش� ���ҽ��� �����ϴ� Pool�� ������ �������� �ȴ�.
		if (Pool->SupportsAllocation(InHeapType, InHeapFlags, InResourceDesc.Flags, InCreateState, InResourceStateMode, Alignment))
		{
			BufferPool = Pool;
			break;
		}
	}

	// ã���� ������ �ش� �Ӽ����� ���� �����.
	if (BufferPool == nullptr)
	{
		BufferPool = CreateBufferPool(InHeapType, InHeapFlags, InResourceDesc.Flags, InResourceStateMode, InInitialResourceState, Alignment);
	}

	// TODO : ���� �ش� Pool�� �Ҵ�޴´�.
	//BufferPool->All
}

Ideal::D3D12PoolAllocator* Ideal::D3D12DefaultBufferAllocator::CreateBufferPool(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, D3D12_RESOURCE_STATES InInitialResourceState, uint32 Alignment)
{
	D3D12ResourceInitConfig InitConfig = {};
	InitConfig.HeapType = InHeapType;
	InitConfig.HeapFlags = InHeapFlags;
	InitConfig.ResourceFlags = InResourceFlags;
	InitConfig.InitialResourceState = InInitialResourceState;

	// ���ҽ��� �Ӽ��� ���� �Ҵ������� ���Ѵ�.
	EResourceAllocationStrategy AllocationStrategy = D3D12PoolAllocator::GetResourceAllocationStrategy(InResourceFlags, InResourceStateMode, Alignment);
	uint64 PoolSize = BUFFER_POOL_DEFAULT_POOL_SIZE; // 32MB BUFFER_POOL_DEFAULT_POOL_SIZE
	uint64 PoolAlignment = (AllocationStrategy == EResourceAllocationStrategy::PlacedResource) ? MIN_PLACED_RESOURCE_SIZE : D3D12ManualSubAllocationAlignment; // 64KB, 256B
	uint64 MaxAllocationSize = BUFFER_POOL_DEFAULT_POOL_MAX_ALLOC_SIZE; // 16MB�� Pool�� �ּ� �ΰ��� �Ҵ��ϰ� �Ѵ�.

	// GPU�� �ִ� �޸𸮰� �ƴϸ� ���� ������ ���� �ʴ´�.
	bool bDefragEnabled = (InitConfig.HeapType == D3D12_HEAP_TYPE_DEFAULT);

	// RTAS�� ��� ���� ������ ���� �ʴ´�.
	// ó�� ���� �� GPU�� Virtual Address�� ������ BuildRaytracingAccelerationStructure�� �̿��Ͽ� �����ϱ� �����̴�.
	// �� �ش� ��ġ�� ��� �����Ѵ�.
	if (InitConfig.InitialResourceState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
	{
		bDefragEnabled = false;
	}

	D3D12PoolAllocator* NewPool = new D3D12PoolAllocator(Device, InitConfig, AllocationStrategy, PoolSize, PoolAlignment, MaxAllocationSize, bDefragEnabled);
	DefaultBufferPools.push_back(NewPool);
	return NewPool;
}

