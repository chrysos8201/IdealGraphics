#include "RHIPoolAllocator.h"
#include "RHIPoolAllocationData.h"
#include "RHIMemoryPool.h"
#include "D3D12\D3D12Util.h"

Ideal::RHIPoolAllocator::RHIPoolAllocator(uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled)
	: DefaultPoolSize(InDefaultPoolSize),
	PoolAlignment(InPoolAlignment),
	MaxAllocationSize(InMaxAllocationSize),
	bDefragEnabled(bInDefragEnabled)
{

}

bool Ideal::RHIPoolAllocator::TryAllocateInternal(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& InAllocationData)
{
	// TODO : 현재 필요한 리소스의 속성과 맞는 Pool을 가져와야함. -> 이미 Pool ALlocator에서 한번 거를 수 있음.
	// 만약 없으면 Pool을 새로 만들고 거기서 할당

	Check((PoolAllocationOrder.size() == Pools.size()));

	for (uint32 PoolIndex : PoolAllocationOrder)
	{
		RHIMemoryPool* Pool = Pools[PoolIndex];
		// 해당 Pool에서 할당할 수 있는지 확인
		if (Pool != nullptr && !Pool->IsFull() && Pool->TryAllocate(InSizeInBytes, InAllocationAlignment, InAllocationData))
		{
			// 여기서 Pool Allocator 시도 하고 성공하면 true 반환.
			// 시도하면서 내부에서 AllocationData를 채워야 함.
			// 성공하면 여기서 함수를 끝낸다.
			return true;
		}
	}

	// 여기서 조각 모음하며 해제됐던 Pool이면 해당 Pool이 nullptr일 것이므로 다시 해당 PoolIndex로 만든다.
	int16 NewPoolIndex = -1;
	for (int32 PoolIndex = 0; PoolIndex < Pools.size(); PoolIndex++)
	{
		if (Pools[PoolIndex] == nullptr)
		{
			NewPoolIndex = PoolIndex;
			break;
		}
	}

	uint32 AlignedSize = RHIMemoryPool::GetAlignedSize(InSizeInBytes, PoolAlignment, InAllocationAlignment);
	if (NewPoolIndex >= 0)
	{
		// 비어있던 Pool 채우기
		Pools[NewPoolIndex] = CreateNewPool(NewPoolIndex, AlignedSize);
	}
	else
	{
		// 마지막에 채움
		NewPoolIndex = Pools.size();
		Pools.push_back(CreateNewPool(NewPoolIndex, AlignedSize));
		PoolAllocationOrder.push_back(NewPoolIndex);
	}

	// 새로 만든 Pools[NewPoolIndex]에서 AllocationPoolData에 할당한다.
	// 할당 여부에 따라 bool 반환
	return Pools[NewPoolIndex]->TryAllocate(InSizeInBytes, InAllocationAlignment, InAllocationData);
}

Ideal::EResourceAllocationStrategy Ideal::D3D12PoolAllocator::GetResourceAllocationStrategy(D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment)
{
	if (Alignment > D3D12ManualSubAllocationAlignment)
	{
		return EResourceAllocationStrategy::PlacedResource;
	}

	// 리소스의 상태 추적과 변경이 필요하면..
	ED3D12ResourceStateMode ResourceStateMode = InResourceStateMode;
	if (ResourceStateMode == ED3D12ResourceStateMode::Default)
	{
		ResourceStateMode = (InResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) ? ED3D12ResourceStateMode::MultiState : ED3D12ResourceStateMode::SingleState;
	}

	// 리소스가 다양한 상태를 가질 경우 ID3D12Heap에서 ID3D12Resource를 할당받게 한다. 
	// 만약 ManualSubAllocation인 경우 하나의 ID3D12Resource를 잘라서 사용하게 될텐데 이 경우 이 리소스를 사용하는 모든 곳에서 상태를 공유하게 된다.
	return (ResourceStateMode == ED3D12ResourceStateMode::MultiState) ? EResourceAllocationStrategy::PlacedResource : EResourceAllocationStrategy::ManualSubAllocation;

}

Ideal::D3D12PoolAllocator::D3D12PoolAllocator(ID3D12Device* InDevice, const D3D12ResourceInitConfig& InInitConfig, EResourceAllocationStrategy InAllocationStrategy, uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled) : D3D12DeviceChild(InDevice),
	RHIPoolAllocator(InDefaultPoolSize, InPoolAlignment, InMaxAllocationSize, bInDefragEnabled),
	InitConfig(InInitConfig),
	AllocationStrategy(InAllocationStrategy)
{

}

bool Ideal::D3D12PoolAllocator::SupportsAllocation(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, D3D12_RESOURCE_STATES InInitialResourceState, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment) const
{
	D3D12ResourceInitConfig InInitConfig = {};
	InInitConfig.HeapType = InHeapType;
	InInitConfig.HeapFlags = InHeapFlags;
	InInitConfig.ResourceFlags = InResourceFlags;
	InInitConfig.InitialResourceState = InInitialResourceState;

	EResourceAllocationStrategy InAllocationStrategy = D3D12PoolAllocator::GetResourceAllocationStrategy(InResourceFlags, InResourceStateMode, Alignment);

	// PlacedResource인 경우 Resource flag와 state는 검사하지 않는다. heap만 체크한다.
	// -> 각각 리소스의 상태는 다를 수 있기 때문에
	if (InAllocationStrategy == EResourceAllocationStrategy::PlacedResource && AllocationStrategy == InAllocationStrategy)
	{
		return (InInitConfig.HeapType == InInitConfig.HeapType && InInitConfig.HeapFlags == InInitConfig.HeapFlags);
	}
	else
	{
		return (InInitConfig == InInitConfig && AllocationStrategy == InAllocationStrategy);
	}
}

void Ideal::D3D12PoolAllocator::AllocateDefaultResource(D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& InResourceDesc, const D3D12_RESOURCE_STATES InResourceCreateState, ED3D12ResourceStateMode InResourceStateMode, uint32 InAllocationAlignment, const D3D12_CLEAR_VALUE* InClearValue, Ideal::D3D12ResourceLocation& ResourceLocation)
{
	uint32 InSize = InResourceDesc.Width;

	if (InHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		Check(InResourceCreateState == D3D12_RESOURCE_STATE_COPY_DEST);
	}
	else if (InHeapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		Check(InResourceCreateState == D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	else if (InResourceCreateState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
	{
		// RTAS의 경우 하나의 상태만 가져야 한다.
		Check(InResourceStateMode == ED3D12ResourceStateMode::SingleState);
	}

	// AllocateResource
	ResourceLocation.Clear();
	if (InSize == 0)
	{
		return;
	}

	bool bPoolResource = InSize <= MaxAllocationSize;
	bool bPlacedResource = bPoolResource && (AllocationStrategy == EResourceAllocationStrategy::PlacedResource);
	if (bPlacedResource)
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc
		// MSAA의 경우 정렬이 4MB이다. 
		// D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT의 경우 64KB인데 
		// 기본으로 Pool을 만들때 설정했던 Alignment보다 커지기 때문
		if (InAllocationAlignment > PoolAlignment || InResourceDesc.Alignment > PoolAlignment)
		{
			bPoolResource = false;
			bPlacedResource = false;
		}
	}

	if (bPoolResource)
	{
		uint32 AllocationAlignment = InAllocationAlignment;

		// 속성이 맞는 pool에서 할당받게 확실히 한다.
		if (bPlacedResource)
		{
			// ManualSubAllocation의 경우 ID3D12Resource를 내부에서 잘라 사용하니 추가적인 Offset이 필요하지만
			// PlacedResource인 경우 이미 ID3D12Heap 내에서 올바르게 정렬된 시작 주소를 가지고 있다.
			// 즉 Place로 생성된 ID3D12Resource 내부에서 Offset을 이용 및 계산하여 view를 만들 필요가 없다(offset은 0이다).
			AllocationAlignment = PoolAlignment;
		}
		else
		{
			// ManualSubAllocation인 경우 할당할 리소스의 플래그와 일치해야 한다.
			// Read Only 리소스의 경우 큰 ID3D12Resource 내부에서 SubAllocate하는데 이는 Resource State를 내부에서 할당된 다른 Resource State와 공유한다는 뜻이다.
			// ex. PoolAllocator의 BackingResource가 항상 SRV로만 사용되도록 D3D12_RESOURCE_FLAG_NONE으로 설정하였는데
			// 이를 렌더 타겟으로 할당하려하면 API단에서 안된다.
			Check(InResourceDesc.Flags == InitConfig.ResourceFlags);
		}

		RHIPoolAllocationData& AllocationData = ResourceLocation.GetPoolAllocatorData();
		TryAllocateInternal(InSize, AllocationAlignment, AllocationData);

		ResourceLocation.SetType(D3D12ResourceLocation::ResourceLocationType::eSubAllocation);
		ResourceLocation.SetPoolAllocator(this);
		ResourceLocation.SetSize(InSize);

		AllocationData.SetOwner(&ResourceLocation);

		if (AllocationStrategy == EResourceAllocationStrategy::ManualSubAllocation)
		{
			ID3D12Resource* BackingResource = GetBackingResource(ResourceLocation);

			ResourceLocation.SetOffsetFromBaseOfResource(AllocationData.GetOffset());
			ResourceLocation.SetResource(BackingResource);
			ResourceLocation.SetGPUVirtualAddress(BackingResource->GetGPUVirtualAddress() + AllocationData.GetOffset());
	
			if (IsCPUAccessible(InitConfig.HeapType))
			{
				// TEST. 아직 사용 X
				void* ResourceBaseAdress;
				Check(BackingResource->Map(0, nullptr, &ResourceBaseAdress));
				ResourceLocation.SetMappedBaseAddress((uint8*)ResourceBaseAdress + AllocationData.GetOffset());
			}
		}
		else
		{
			Check(ResourceLocation.GetResource() == nullptr);

			D3D12_RESOURCE_DESC Desc = InResourceDesc;
			Desc.Alignment = AllocationAlignment;

			ID3D12Resource* NewResource = CreatePlacedResource(AllocationData, Desc, InResourceCreateState, InResourceStateMode, InClearValue);
			ResourceLocation.SetResource(NewResource);
			ResourceLocation.SetGPUVirtualAddress(NewResource->GetGPUVirtualAddress());
		}
	}
}

ID3D12Resource* Ideal::D3D12PoolAllocator::GetBackingResource(D3D12ResourceLocation& InResourceLocation) const
{
	Check(IsOwner(InResourceLocation));

	RHIPoolAllocationData& AllocationData = InResourceLocation.GetPoolAllocatorData();
	return ((D3D12MemoryPool*)Pools[AllocationData.GetPoolIndex()])->GetBackingResource();
}

Ideal::RHIMemoryPool* Ideal::D3D12PoolAllocator::CreateNewPool(uint32 InPoolIndex, uint32 InMinimumAllocationSize)
{
	uint32 PoolSize = DefaultPoolSize;

	if (InMinimumAllocationSize > PoolSize)
	{
		Check(InMinimumAllocationSize <= MaxAllocationSize);
	
		unsigned long BitIndex = 0;
		// 리소스의 사이즈보다 크거나 같은 2의 거듭제곱 형태로 나타낸다.
		// 이진수를 MSB부터 탐색하여 가장 왼쪽에서부터 몇번째에 있는지 찾아낸다.
		// ex) 8 = 0b0100 -> (8 - 1) * 2 + 1 = 15, 0b111 -> alignedsize = 8
		_BitScanReverse(&BitIndex, (InMinimumAllocationSize - 1) * 2 + 1);
		uint32 NewAlignedSIze = 1 << BitIndex;
		PoolSize = std::min<uint32>(NewAlignedSIze, (uint32)MaxAllocationSize);
	}

	D3D12MemoryPool* NewPool = new D3D12MemoryPool(Device, InPoolIndex, PoolSize, PoolAlignment, AllocationStrategy, InitConfig);
	NewPool->Init();
	return NewPool;
}

ID3D12Resource* Ideal::D3D12PoolAllocator::CreatePlacedResource(const RHIPoolAllocationData& InAllocationData, const D3D12_RESOURCE_DESC& InDesc, D3D12_RESOURCE_STATES InCreateState, ED3D12ResourceStateMode InResourceStateMode, const D3D12_CLEAR_VALUE* InClearValue)
{
	ID3D12Heap* Heap = ((D3D12MemoryPool*)Pools[InAllocationData.GetPoolIndex()])->GetBackingHeap();
	uint64 Offset = uint64(AlignDown(InAllocationData.GetOffset(), PoolAlignment));

	ID3D12Resource* NewResource = nullptr;
	Device->CreatePlacedResource(Heap, Offset, &InDesc, InCreateState, InClearValue, IID_PPV_ARGS(&NewResource));
	return NewResource;
}
