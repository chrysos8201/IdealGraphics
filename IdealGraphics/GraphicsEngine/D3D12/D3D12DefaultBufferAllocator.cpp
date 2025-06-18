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
		// 해당 리소스를 지원하는 Pool이 있으면 꺼내오면 된다.
		if (Pool->SupportsAllocation(InHeapType, InHeapFlags, InResourceDesc.Flags, InCreateState, InResourceStateMode, Alignment))
		{
			BufferPool = Pool;
			break;
		}
	}

	// 찾은게 없으면 해당 속성으로 새로 만든다.
	if (BufferPool == nullptr)
	{
		BufferPool = CreateBufferPool(InHeapType, InHeapFlags, InResourceDesc.Flags, InResourceStateMode, InInitialResourceState, Alignment);
	}

	// TODO : 이제 해당 Pool로 할당받는다.
	//BufferPool->All
}

Ideal::D3D12PoolAllocator* Ideal::D3D12DefaultBufferAllocator::CreateBufferPool(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, D3D12_RESOURCE_STATES InInitialResourceState, uint32 Alignment)
{
	D3D12ResourceInitConfig InitConfig = {};
	InitConfig.HeapType = InHeapType;
	InitConfig.HeapFlags = InHeapFlags;
	InitConfig.ResourceFlags = InResourceFlags;
	InitConfig.InitialResourceState = InInitialResourceState;

	// 리소스의 속성에 따라 할당전략을 정한다.
	EResourceAllocationStrategy AllocationStrategy = D3D12PoolAllocator::GetResourceAllocationStrategy(InResourceFlags, InResourceStateMode, Alignment);
	uint64 PoolSize = BUFFER_POOL_DEFAULT_POOL_SIZE; // 32MB BUFFER_POOL_DEFAULT_POOL_SIZE
	uint64 PoolAlignment = (AllocationStrategy == EResourceAllocationStrategy::PlacedResource) ? MIN_PLACED_RESOURCE_SIZE : D3D12ManualSubAllocationAlignment; // 64KB, 256B
	uint64 MaxAllocationSize = BUFFER_POOL_DEFAULT_POOL_MAX_ALLOC_SIZE; // 16MB로 Pool에 최소 두개는 할당하게 한다.

	// GPU에 있는 메모리가 아니면 조각 모음을 하지 않는다.
	bool bDefragEnabled = (InitConfig.HeapType == D3D12_HEAP_TYPE_DEFAULT);

	// RTAS의 경우 조각 모음을 하지 않는다.
	// 처음 만들 때 GPU의 Virtual Address를 가지고 BuildRaytracingAccelerationStructure를 이용하여 빌드하기 때문이다.
	// 즉 해당 위치를 계속 유지한다.
	if (InitConfig.InitialResourceState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
	{
		bDefragEnabled = false;
	}

	D3D12PoolAllocator* NewPool = new D3D12PoolAllocator(Device, InitConfig, AllocationStrategy, PoolSize, PoolAlignment, MaxAllocationSize, bDefragEnabled);
	DefaultBufferPools.push_back(NewPool);
	return NewPool;
}

