#include "RHIPoolAllocator.h"
#include "RHIPoolAllocationData.h"
#include "RHIMemoryPool.h"
#include "D3D12\D3D12Util.h"
#include "d3dx12.h"
#include "D3D12/D3D12Resource.h"

static float GRHIPoolAllocatorDefragSizeFraction = 0.9f;
static int32 GRHIPoolAllocatorDefragMaxPoolsToClear = 1;

Ideal::RHIPoolAllocator::RHIPoolAllocator(uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled)
	: DefaultPoolSize(InDefaultPoolSize),
	LastDefragPoolIndex(0),
	PoolAlignment(InPoolAlignment),
	MaxAllocationSize(InMaxAllocationSize),
	bDefragEnabled(bInDefragEnabled)
{

}

Ideal::RHIPoolAllocator::~RHIPoolAllocator()
{

}

void Ideal::RHIPoolAllocator::Defrag(const RHIContext& Context, uint32 InMaxCopySize, uint32& CurrentCopySize)
{
	if (!bDefragEnabled)
	{
		return;
	}

	std::vector<RHIMemoryPool*> SortedTargetPools;
	SortedTargetPools.reserve(Pools.size());
	uint32 MaxPoolsToDefrag = 0;
	for (RHIMemoryPool* Pool : Pools)
	{
		// ä������ ���� pool�� ����
		if (Pool && !Pool->IsFull())
		{
			SortedTargetPools.push_back(Pool);
		}

		float Usage = Pool ? (float)Pool->GetUsedSize() / (float)Pool->GetPoolSize() : 1.0f;
		if (Usage < GRHIPoolAllocatorDefragSizeFraction)
		{
			MaxPoolsToDefrag++;
		}
	}

	// ���� ������ �ʿ��� MemoryPool�� ���� 0�̰ų� ��� ������ MemoryPool�� ����
	// 2���� ������ ���� ������ ������� �ʴ´�.
	if (MaxPoolsToDefrag == 0 || SortedTargetPools.size() < 2)
	{
		return;
	}

	std::sort(SortedTargetPools.begin(), SortedTargetPools.end(),
		[](const RHIMemoryPool* InLHS, const RHIMemoryPool* InRHS)
		{
			return InLHS->GetUsedSize() < InRHS->GetUsedSize();
		});

	std::vector<RHIMemoryPool*> TargetPools;

	// GRHIPoolAllocatorDefragMaxPoolsToClear : �����Ӵ� �󸶳� ���� MemoryPool�� ���� ������ �� �ִ����� ���� ���� 
	if (GRHIPoolAllocatorDefragMaxPoolsToClear < 0)
	{
		// ���� ������ ��ǥ�� �� ���Ǵ� MemoryPool ������ �� ���� ���Ǵ� MemoryPool�� �����ϴ� ���̴�.
		// ���⼭ ���� ������ ���� ��� �޸� Ǯ TargetPools�� �����ǰ� �޸� Ǯ�� ��뷮 ������ ���ĵȴ�.
		// SortedTargetPools�� 0��° MemoryPool�� ���� ������ ù ��° �޸� Ǯ�̱� ������ �ǳʶڴ�.
		TargetPools.reserve(SortedTargetPools.size());
		for (int32 PoolIndex = SortedTargetPools.size() - 1; PoolIndex > 0; --PoolIndex)
		{
			TargetPools.push_back(SortedTargetPools[PoolIndex]);
		}

		// ���� ������ �ʿ��� �޸� Ǯ Ž��
		for (int32 PoolIndex = 0; PoolIndex < SortedTargetPools.size() - 1; ++PoolIndex)
		{
			RHIMemoryPool* PoolToClear = SortedTargetPools[PoolIndex];
			PoolToClear->TryClear(Context, this, InMaxCopySize, CurrentCopySize, TargetPools);

			TargetPools.pop_back();

			if (CurrentCopySize >= InMaxCopySize)
			{
				break;
			}
		}
	}
	else
	{
		if (LastDefragPoolIndex >= Pools.size())
		{
			LastDefragPoolIndex = 0;
		}
		int32 DefraggedPoolCount = 0;
		for (; LastDefragPoolIndex < Pools.size(); LastDefragPoolIndex++)
		{
			RHIMemoryPool* PoolToClear = Pools[LastDefragPoolIndex];

			// invalid�̰ų� ������ ���� ���� �̻��̸� �Ѿ��.
			float Usage = PoolToClear ? (float)PoolToClear->GetUsedSize() / (float)PoolToClear->GetPoolSize() : 1.0f;
			if (Usage >= GRHIPoolAllocatorDefragSizeFraction)
			{
				continue;
			}

			TargetPools.clear();
			for (int32 SortedPoolIndex = SortedTargetPools.size() - 1; SortedPoolIndex >= 0; --SortedPoolIndex)
			{
				RHIMemoryPool* TargetPool = SortedTargetPools[SortedPoolIndex];
				if (PoolToClear == TargetPool)
				{
					break;
				}
				TargetPools.push_back(TargetPool);
			}

			if (TargetPools.size() == 0)
			{
				continue;
			}

			PoolToClear->TryClear(Context, this, InMaxCopySize, CurrentCopySize, TargetPools);
			DefraggedPoolCount++;
			if (CurrentCopySize >= InMaxCopySize || (DefraggedPoolCount >= GRHIPoolAllocatorDefragMaxPoolsToClear))
			{
				break;
			}
		}
	}
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

void Ideal::RHIPoolAllocator::DeallocateInternal(RHIPoolAllocationData& AllocationData)
{
	Check(AllocationData.IsAllocated());
	Check(AllocationData.GetPoolIndex() < Pools.size());
	Pools[AllocationData.GetPoolIndex()]->Deallocate(AllocationData);
}

void Ideal::RHIPoolAllocator::SortPools()
{
	std::sort(PoolAllocationOrder.begin(), PoolAllocationOrder.end(), [this](uint32 InLHS, uint32 InRHS)
		{
			if (bDefragEnabled)
			{
				uint32 LHSUsePoolSize = Pools[InLHS] ? Pools[InLHS]->GetUsedSize() : UINT32_MAX;
				uint32 RHSUSePoolSize = Pools[InRHS] ? Pools[InRHS]->GetUsedSize() : UINT32_MAX;
				return LHSUsePoolSize > RHSUSePoolSize;
			}
			else
			{
				uint32 LHSPoolSize = Pools[InLHS] ? Pools[InLHS]->GetPoolSize() : UINT32_MAX;
				uint32 RHSPoolSize = Pools[InRHS] ? Pools[InRHS]->GetPoolSize() : UINT32_MAX;
				if (LHSPoolSize != RHSPoolSize)
				{
					return LHSPoolSize < RHSPoolSize;
				}

				uint32 LHSPoolIndex = Pools[InLHS] ? Pools[InLHS]->GetPoolIndex() : UINT32_MAX;
				uint32 RHSPoolIndex = Pools[InRHS] ? Pools[InRHS]->GetPoolIndex() : UINT32_MAX;
				return LHSPoolIndex < RHSPoolIndex;
			}
		});
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

void Ideal::D3D12PoolAllocator::BeginAndSetFenceValue(uint64 InFenceValue)
{
	FenceValue = InFenceValue;
}

Ideal::D3D12PoolAllocator::D3D12PoolAllocator(ID3D12Device* InDevice, const D3D12ResourceInitConfig& InInitConfig, EResourceAllocationStrategy InAllocationStrategy, uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled) 
	: D3D12DeviceChild(InDevice),
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

	ResourceLocation.SetResourceStateMode(InResourceStateMode);

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
			ResourceLocation.SetAllocationStrategy(EResourceAllocationStrategy::ManualSubAllocation);
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
			ResourceLocation.SetAllocationStrategy(EResourceAllocationStrategy::PlacedResource);
			ResourceLocation.SetGPUVirtualAddress(NewResource->GetGPUVirtualAddress());
		}
	}
	else
	{
		ID3D12Resource* NewResource = nullptr;
		const D3D12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InHeapType);
		D3D12_RESOURCE_DESC Desc = InResourceDesc;
		Desc.Alignment = 0;

		D3D12_RESOURCE_STATES DefaultState = InResourceCreateState;
		Device->CreateCommittedResource(
			&HeapProps,
			D3D12_HEAP_FLAG_NONE,
			&Desc,
			InResourceCreateState,
			nullptr,
			IID_PPV_ARGS(&NewResource)
			);

		ResourceLocation.AsStandAlone(NewResource, HeapProps.Type ,InSize);
	}
}

void Ideal::D3D12PoolAllocator::DeallocateResource(D3D12ResourceLocation& ResourceLocation, bool bDefragFree /*= false*/)
{
	Check(IsOwner(ResourceLocation));

	RHIPoolAllocationData& AllocationData = ResourceLocation.GetPoolAllocatorData();
	Check(AllocationData.IsAllocated());

	if (AllocationData.IsLocked())
	{
		for (FrameFencedAllocationData& Operation : FrameFencedOperations)
		{
			if (Operation.AllocationData == &AllocationData)
			{
				Check(Operation.Operation == FrameFencedAllocationData::EOperation::Unlock);
				Operation.Operation = FrameFencedAllocationData::EOperation::Nop;
				Operation.AllocationData = nullptr;
				break;
			}
		}

		// ���� ���� ���μ������� ������� ���� ���� �޸� ���� �۾��� ����Ѵ�.
		for (D3D12VRAMCopyOperation& CopyOperation : PendingCopyOps)
		{
			if (CopyOperation.SourceResource == ResourceLocation.GetResource() ||
				CopyOperation.DestResource == ResourceLocation.GetResource())
			{
				CopyOperation.SourceResource = nullptr;
				CopyOperation.DestResource = nullptr;
				break;
			}
		}
	}

	int16 PoolIndex = AllocationData.GetPoolIndex();

	RHIPoolAllocationData* ReleasedAllocationData = nullptr; // (AllocationDataPool.size() > 0) ? AllocationDataPool.back()
	if (AllocationDataPool.size() > 0)
	{
		ReleasedAllocationData = AllocationDataPool[AllocationDataPool.size() - 1];
		AllocationDataPool.pop_back();
	}
	else
	{
		ReleasedAllocationData = new RHIPoolAllocationData();
	}

	bool bLocked = true;
	ReleasedAllocationData->MoveFrom(AllocationData, bLocked);

	ResourceLocation.ClearAllocator();

	FrameFencedAllocationData DeleteRequest;
	DeleteRequest.Operation = FrameFencedAllocationData::EOperation::Deallocate;
	DeleteRequest.FrameFence = FenceValue;
	DeleteRequest.AllocationData = ReleasedAllocationData;

	PendingDeleteRequestSize += DeleteRequest.AllocationData->GetSize();
	((D3D12MemoryPool*)Pools[PoolIndex])->UpdateLastUsedFrameFence(DeleteRequest.FrameFence);

	// PlacedResource�� ��� Fence�� ������ ������ �� �ְ� ���� // ManualResource�� ��� �ϳ��� Resource�� �߶� ���� ����X
	if (ResourceLocation.GetAllocationStrategy() == EResourceAllocationStrategy::PlacedResource)
	{
		DeleteRequest.PlacedResource = ResourceLocation.GetResource();
	}
	else
	{
		DeleteRequest.PlacedResource = nullptr;
	}

	FrameFencedOperations.push_back(DeleteRequest);
}

void Ideal::D3D12PoolAllocator::CleanUpAllocations(uint64 InFrameLag, bool bForceFree /*= false*/)
{
	uint32 PopCount = 0;
	for (int32 i = 0; i < FrameFencedOperations.size(); i++)
	{
		FrameFencedAllocationData& Operation = FrameFencedOperations[i];
		if (bForceFree || Operation.FrameFence <= FenceValue)
		{
			switch (Operation.Operation)
			{
				case FrameFencedAllocationData::EOperation::Deallocate:
				{
					Check(PendingDeleteRequestSize >= Operation.AllocationData->GetSize());
					PendingDeleteRequestSize -= Operation.AllocationData->GetSize();

					DeallocateInternal(*Operation.AllocationData);
					Operation.AllocationData->Reset();
					AllocationDataPool.push_back(Operation.AllocationData);

					if (AllocationStrategy == EResourceAllocationStrategy::PlacedResource)
					{
						Check(Operation.PlacedResource != nullptr);
						Operation.PlacedResource->Release();
						Operation.PlacedResource = nullptr;
					}
					else
					{
						Check(Operation.PlacedResource == nullptr);
					}
					break;
				}
				case FrameFencedAllocationData::EOperation::Unlock:
				{
					Operation.AllocationData->UnLock();
					break;
				}
				case FrameFencedAllocationData::EOperation::Nop:
					break;
				default:
					Check(false);
					break;
			}

			PopCount = i + 1;
		}
		else
		{
			break;
		}
	}

	if (PopCount)
	{
		FrameFencedOperations.erase(FrameFencedOperations.begin(), FrameFencedOperations.begin() + PopCount);
	}

	// ���� N�����ӵ��� ������� ���� �� Allocator���� �����ϰ� �����ؾ� �Ѵ�.
	for (int32 PoolIndex = 0; PoolIndex < Pools.size(); ++PoolIndex)
	{
		D3D12MemoryPool* MemoryPool = (D3D12MemoryPool*)Pools[PoolIndex];
		if (MemoryPool != nullptr && MemoryPool->IsEmpty() && (bForceFree || (MemoryPool->GetLastUsedFrameFence() + InFrameLag <= FenceValue)))
		{
			MemoryPool->Destroy();
			delete(MemoryPool);
			Pools[PoolIndex] = nullptr;
		}
	}

	SortPools();
}

void Ideal::D3D12PoolAllocator::FlushPendingCopyOps(D3D12Context& InContext)
{
	for (D3D12VRAMCopyOperation& CopyOperation : PendingCopyOps)
	{
		if (CopyOperation.SourceResource == nullptr || CopyOperation.DestResource == nullptr)
		{
			continue;
		}

		{
			CD3DX12_RESOURCE_BARRIER barrierSource = CD3DX12_RESOURCE_BARRIER::Transition(
				CopyOperation.SourceResource,
				CopyOperation.SourceResourceState,
				D3D12_RESOURCE_STATE_COPY_SOURCE
			);
			CD3DX12_RESOURCE_BARRIER barrierDest = CD3DX12_RESOURCE_BARRIER::Transition(
				CopyOperation.DestResource,
				CopyOperation.DestResourceState,
				D3D12_RESOURCE_STATE_COPY_DEST
			);
			CD3DX12_RESOURCE_BARRIER Barriers[2] = { barrierSource, barrierDest };
			InContext.CommandList->ResourceBarrier(2, Barriers);

			switch (CopyOperation.CopyType)
			{
				case D3D12VRAMCopyOperation::BufferRegion:
				{
					InContext.CommandList->CopyBufferRegion(
						CopyOperation.DestResource,
						CopyOperation.DestOffset,
						CopyOperation.SourceResource,
						CopyOperation.SourceOffset,
						CopyOperation.Size
					);
					break;
				}
				case D3D12VRAMCopyOperation::Resource:
				{
					InContext.CommandList->CopyResource(
						CopyOperation.DestResource,
						CopyOperation.SourceResource
					);
					break;
				}
			}

			CD3DX12_RESOURCE_BARRIER barrierDestToResource = CD3DX12_RESOURCE_BARRIER::Transition(
				CopyOperation.DestResource,
				D3D12_RESOURCE_STATE_COPY_DEST,
				CopyOperation.SourceResourceState
			);

			InContext.CommandList->ResourceBarrier(1, &barrierDestToResource);
		}
	}

	PendingCopyOps.clear();
}

ID3D12Resource* Ideal::D3D12PoolAllocator::GetBackingResource(D3D12ResourceLocation& InResourceLocation) const
{
	Check(IsOwner(InResourceLocation));

	RHIPoolAllocationData& AllocationData = InResourceLocation.GetPoolAllocatorData();
	return ((D3D12MemoryPool*)Pools[AllocationData.GetPoolIndex()])->GetBackingResource();
}

Ideal::D3D12HeapAndOffset Ideal::D3D12PoolAllocator::GetBackingHeapAndAllocationOffsetInBytes(D3D12ResourceLocation& InResourceLocation) const
{
	Check(IsOwner(InResourceLocation));
	return GetBackingHeapAndAllocationOffsetInBytes(InResourceLocation.GetPoolAllocatorData());
}

bool Ideal::D3D12PoolAllocator::IsOwner(D3D12ResourceLocation& ResourceLocation) const
{
	return ResourceLocation.GetPoolAllocator() == this;
}

Ideal::D3D12HeapAndOffset Ideal::D3D12PoolAllocator::GetBackingHeapAndAllocationOffsetInBytes(const RHIPoolAllocationData& InAllocationData) const
{
	D3D12HeapAndOffset HeapAndOffset;
	HeapAndOffset.Heap = ((D3D12MemoryPool*)Pools[InAllocationData.GetPoolIndex()])->GetBackingHeap();
	HeapAndOffset.Offset = uint64(AlignDown(InAllocationData.GetOffset(), PoolAlignment));
	return HeapAndOffset;
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

bool Ideal::D3D12PoolAllocator::HandleDefragRequest(const RHIContext& Context, RHIPoolAllocationData* InSourceBlock, RHIPoolAllocationData& InTmpTargetBlock)
{
	// InSourceBlock Info
	D3D12ResourceLocation* Owner = (D3D12ResourceLocation*)InSourceBlock->GetOwner();
	uint64 CurrentOffset = Owner->GetOffsetFromBaseOfResource();
	ID3D12Resource* CurrentResource = Owner->GetResource();

	D3D12_RESOURCE_STATES SourceResourceState = Owner->GetOwner().lock()->GetResourceState();;

	bool bDefragFree = true;
	DeallocateResource(*Owner, bDefragFree);

	// InTmpTargetBlock�� �����͸� InSourceBlock���� �����ϰ� InSourceBlock�� ��ٴ�.
	bool bLocked = true;
	InSourceBlock->MoveFrom(InTmpTargetBlock, bLocked);
	InSourceBlock->SetOwner(Owner);
	Owner->SetPoolAllocator(this);

	// TODO : Copy Operator���� �� �ؾ���.
	// -> �̷��� �Ǹ� BLAS���� ����ϴ� �͵鵵 �ٽ� �������־�� �Ѵ�. // 07.08
	D3D12_RESOURCE_STATES DestCreateState = D3D12_RESOURCE_STATE_COMMON;
	Owner->OnAllocationMoved((D3D12Context&)Context, InSourceBlock, DestCreateState);

	if (Owner->GetResourceStateMode() == ED3D12ResourceStateMode::MultiState)
	{
		// Tracking
		DestCreateState = D3D12_RESOURCE_STATE_COMMON;
	}
	else
	{
		DestCreateState = SourceResourceState;
	}

	FrameFencedAllocationData UnlockRequest = {};
	UnlockRequest.Operation = FrameFencedAllocationData::EOperation::Unlock;
	UnlockRequest.FrameFence = FenceValue;
	UnlockRequest.AllocationData = InSourceBlock;
	FrameFencedOperations.push_back(UnlockRequest);

	// InSourceBlock�� ���� ���ҽ����� InTmpTargetBlock�� ��� ���ҽ��� ���� �޸� ���� �۾� �߰�
	// �� ���� �۾��� PoolAllocator�� ���� ���� ������ ���� ���� ����ȴ�.
	D3D12VRAMCopyOperation CopyOp;
	CopyOp.SourceResource = CurrentResource;
	CopyOp.SourceResourceState = SourceResourceState;
	CopyOp.SourceOffset = CurrentOffset;
	CopyOp.DestResource = Owner->GetResource();
	CopyOp.DestResourceState = DestCreateState;	// �ϴ� ����Ǹ� ���� ���ҽ��� State�� ���� �����ϵ��� �ΰڴ�(25.07.10)
	CopyOp.DestOffset = Owner->GetOffsetFromBaseOfResource();
	CopyOp.Size = InSourceBlock->GetSize();
	CopyOp.CopyType = AllocationStrategy == EResourceAllocationStrategy::ManualSubAllocation ? D3D12VRAMCopyOperation::ECopyType::BufferRegion : D3D12VRAMCopyOperation::ECopyType::Resource;
	Check(CopyOp.SourceResource != nullptr);
	Check(CopyOp.DestResource != nullptr);
	PendingCopyOps.push_back(CopyOp);

	// 25.07.13 Defrag Dirty
	Owner->SetDefragDirtyCheckForBLAS(true);

	return true;
}
