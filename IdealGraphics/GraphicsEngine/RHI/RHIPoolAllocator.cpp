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
	// TODO : ���� �ʿ��� ���ҽ��� �Ӽ��� �´� Pool�� �����;���. -> �̹� Pool ALlocator���� �ѹ� �Ÿ� �� ����.
	// ���� ������ Pool�� ���� ����� �ű⼭ �Ҵ�

	Check((PoolAllocationOrder.size() == Pools.size()));

	for (uint32 PoolIndex : PoolAllocationOrder)
	{
		RHIMemoryPool* Pool = Pools[PoolIndex];
		// �ش� Pool���� �Ҵ��� �� �ִ��� Ȯ��
		if (Pool != nullptr && !Pool->IsFull() && Pool->TryAllocate(InSizeInBytes, InAllocationAlignment, InAllocationData))
		{
			// ���⼭ Pool Allocator �õ� �ϰ� �����ϸ� true ��ȯ.
			// �õ��ϸ鼭 ���ο��� AllocationData�� ä���� ��.
			// �����ϸ� ���⼭ �Լ��� ������.
			return true;
		}
	}

	// ���⼭ ���� �����ϸ� �����ƴ� Pool�̸� �ش� Pool�� nullptr�� ���̹Ƿ� �ٽ� �ش� PoolIndex�� �����.
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
		// ����ִ� Pool ä���
		Pools[NewPoolIndex] = CreateNewPool(NewPoolIndex, AlignedSize);
	}
	else
	{
		// �������� ä��
		NewPoolIndex = Pools.size();
		Pools.push_back(CreateNewPool(NewPoolIndex, AlignedSize));
		PoolAllocationOrder.push_back(NewPoolIndex);
	}

	// ���� ���� Pools[NewPoolIndex]���� AllocationPoolData�� �Ҵ��Ѵ�.
	// �Ҵ� ���ο� ���� bool ��ȯ
	return Pools[NewPoolIndex]->TryAllocate(InSizeInBytes, InAllocationAlignment, InAllocationData);
}

Ideal::EResourceAllocationStrategy Ideal::D3D12PoolAllocator::GetResourceAllocationStrategy(D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment)
{
	if (Alignment > D3D12ManualSubAllocationAlignment)
	{
		return EResourceAllocationStrategy::PlacedResource;
	}

	// ���ҽ��� ���� ������ ������ �ʿ��ϸ�..
	ED3D12ResourceStateMode ResourceStateMode = InResourceStateMode;
	if (ResourceStateMode == ED3D12ResourceStateMode::Default)
	{
		ResourceStateMode = (InResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) ? ED3D12ResourceStateMode::MultiState : ED3D12ResourceStateMode::SingleState;
	}

	// ���ҽ��� �پ��� ���¸� ���� ��� ID3D12Heap���� ID3D12Resource�� �Ҵ�ް� �Ѵ�. 
	// ���� ManualSubAllocation�� ��� �ϳ��� ID3D12Resource�� �߶� ����ϰ� ���ٵ� �� ��� �� ���ҽ��� ����ϴ� ��� ������ ���¸� �����ϰ� �ȴ�.
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

	// PlacedResource�� ��� Resource flag�� state�� �˻����� �ʴ´�. heap�� üũ�Ѵ�.
	// -> ���� ���ҽ��� ���´� �ٸ� �� �ֱ� ������
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
		// RTAS�� ��� �ϳ��� ���¸� ������ �Ѵ�.
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
		// MSAA�� ��� ������ 4MB�̴�. 
		// D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT�� ��� 64KB�ε� 
		// �⺻���� Pool�� ���鶧 �����ߴ� Alignment���� Ŀ���� ����
		if (InAllocationAlignment > PoolAlignment || InResourceDesc.Alignment > PoolAlignment)
		{
			bPoolResource = false;
			bPlacedResource = false;
		}
	}

	if (bPoolResource)
	{
		uint32 AllocationAlignment = InAllocationAlignment;

		// �Ӽ��� �´� pool���� �Ҵ�ް� Ȯ���� �Ѵ�.
		if (bPlacedResource)
		{
			// ManualSubAllocation�� ��� ID3D12Resource�� ���ο��� �߶� ����ϴ� �߰����� Offset�� �ʿ�������
			// PlacedResource�� ��� �̹� ID3D12Heap ������ �ùٸ��� ���ĵ� ���� �ּҸ� ������ �ִ�.
			// �� Place�� ������ ID3D12Resource ���ο��� Offset�� �̿� �� ����Ͽ� view�� ���� �ʿ䰡 ����(offset�� 0�̴�).
			AllocationAlignment = PoolAlignment;
		}
		else
		{
			// ManualSubAllocation�� ��� �Ҵ��� ���ҽ��� �÷��׿� ��ġ�ؾ� �Ѵ�.
			// Read Only ���ҽ��� ��� ū ID3D12Resource ���ο��� SubAllocate�ϴµ� �̴� Resource State�� ���ο��� �Ҵ�� �ٸ� Resource State�� �����Ѵٴ� ���̴�.
			// ex. PoolAllocator�� BackingResource�� �׻� SRV�θ� ���ǵ��� D3D12_RESOURCE_FLAG_NONE���� �����Ͽ��µ�
			// �̸� ���� Ÿ������ �Ҵ��Ϸ��ϸ� API�ܿ��� �ȵȴ�.
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
				// TEST. ���� ��� X
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
		// ���ҽ��� ������� ũ�ų� ���� 2�� �ŵ����� ���·� ��Ÿ����.
		// �������� MSB���� Ž���Ͽ� ���� ���ʿ������� ���°�� �ִ��� ã�Ƴ���.
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
