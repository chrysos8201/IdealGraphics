#include "RHIMemoryPool.h"
#include "d3d12.h"
#include "d3dx12_core.h"
#include "D3D12\D3D12Common.h"
#include "RHIPoolAllocationData.h"
#include "D3D12\D3D12Definitions.h"
#include "D3D12\D3D12Util.h"


static int32 DesiredAllocationPoolSize = 32;

uint32 Ideal::RHIMemoryPool::GetAlignedSize(uint32 InSizeInBytes, uint32 InPoolAlignment, uint32 InAllocationAlignment)
{
	Check(InAllocationAlignment <= InPoolAlignment);

	uint32 Size = ((InPoolAlignment % InAllocationAlignment) != 0) ? InSizeInBytes + InAllocationAlignment : InSizeInBytes;
	return AlignArbitrary(Size, InPoolAlignment);	// ����
}

uint32 Ideal::RHIMemoryPool::GetAlignedOffset(uint32 InOffset, uint32 InPoolAlignment, uint32 InAllocationAlignment)
{
	uint32 AlignedOffset = InOffset;
	
	if ((InPoolAlignment % InAllocationAlignment) != 0)
	{
		uint32 AlignmentRest = AlignedOffset % InAllocationAlignment;
		if (AlignmentRest > 0)
		{
			AlignedOffset += (InAllocationAlignment - AlignmentRest);
		}
	}

	return AlignedOffset;
}

//using namespace Ideal;

Ideal::RHIMemoryPool::RHIMemoryPool(uint16 InPoolIndex, uint64 InPoolSize, uint32 InPoolAlignment) :
	PoolIndex(InPoolIndex)
	, PoolSize(InPoolSize)
	, PoolAlignment(InPoolAlignment)
	, FreeSize(0)	
	, AlignmentWaste(0)
	, AllocatedBlocks(0)
{

}

void Ideal::RHIMemoryPool::Init()
{
	FreeSize = PoolSize;

	AllocationDataPool.reserve(DesiredAllocationPoolSize);
	for (int32 Index = 0; Index < DesiredAllocationPoolSize; ++Index)
	{
		RHIPoolAllocationData* AllocationData = new RHIPoolAllocationData();
		AllocationData->Reset();
		AllocationDataPool.push_back(AllocationData);
	}

	HeadBlock.InitAsHead(PoolIndex);
	RHIPoolAllocationData* FreeBlock = GetNewAllocationData();
	FreeBlock->InitAsFree(PoolIndex, PoolSize, PoolAlignment, 0);
	HeadBlock.AddAfter(FreeBlock);
	FreeBlocks.push_back(FreeBlock);

	Validate();
}


bool Ideal::RHIMemoryPool::TryAllocate(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& AllocationData)
{
	int32 FreeBlockIndex = FindFreeBlock(InSizeInBytes, InAllocationAlignment);
	if (FreeBlockIndex != -1)
	{
		uint32 AlignedSize = GetAlignedSize(InSizeInBytes, PoolAlignment, InAllocationAlignment);

		RHIPoolAllocationData* FreeBlock = FreeBlocks[FreeBlockIndex];
		Check(FreeBlock->GetSize() >= AlignedSize);

		// �ϴ� ����� �����ͼ� �����Ѵ�. ���� ������ ��Ͽ��� ���� ������ ����� ���߿� �ٽ� �ְ� ������ ���̴�.
		FreeBlocks.erase(FreeBlocks.begin() + FreeBlockIndex);

		// ������ ����� AllocationData�� �ְ� ���� ����� FreeBlock�� ��� �����ִ�.
		AllocationData.InitAsAllocated(InSizeInBytes, PoolAlignment, InAllocationAlignment, FreeBlock);
		Check((AllocationData.GetOffset() % InAllocationAlignment) == 0);
		
		// ���� ������Ʈ
		Check(FreeSize >= AlignedSize);
		FreeSize -= AlignedSize;
		AlignmentWaste += (AlignedSize - InSizeInBytes);
		AllocatedBlocks++;
		

		// ���� ���� ����� ����� 0�� ��� ���� �Ҵ���� ����̶� �� �´´ٴ� ��
		if (FreeBlock->GetSize() == 0)
		{
			// ����� ��ũ�� ����Ʈ���� freeblock �հ� �ڸ� ���� �������ش�.
			FreeBlock->RemoveFromLinkedList();
			ReleaseAllocationData(FreeBlock);

			// Validate
			Validate();
		}
		else
		{
			AddToFreeBlocks(FreeBlock);
		}
		return true;

	}

	return false;
}

void Ideal::RHIMemoryPool::Deallocate(RHIPoolAllocationData& AllocationData)
{
	Check(AllocationData.IsLocked());
	Check(PoolIndex == AllocationData.GetPoolIndex());

	uint32 AllocationAlignment = AllocationData.GetAlignment();

	bool bLocked = false;
	uint64 AllocationSize = AllocationData.GetSize();

	RHIPoolAllocationData* FreeBlock = GetNewAllocationData();
	FreeBlock->MoveFrom(AllocationData, bLocked);
	FreeBlock->MarkFree(PoolAlignment, AllocationAlignment);

	FreeSize += FreeBlock->GetSize();
	AlignmentWaste -= FreeBlock->GetSize() - AllocationSize;
	AllocatedBlocks--;

	AddToFreeBlocks(FreeBlock);
}

int32 Ideal::RHIMemoryPool::FindFreeBlock(uint32 InSizeInBytes, uint32 InAllocationAlignment) const
{
	uint32 AlignedSize = GetAlignedSize(InSizeInBytes, PoolAlignment, InAllocationAlignment);
	
	// MemoryPool���� ����� �� �ִ� ũ�⸦ �Ѿ�� ���� �����Ѵ�.
	if (FreeSize < AlignedSize)
	{
		return -1;
	}

	// ������ ������� �����Ѵ�.

	// FreeBlocks ���� ã�´�.
	// ù ��° ����� ũ�Ⱑ �䱸 ������ �����ϸ�
	// �׻� ù ��° ��尡 ��ȯ�ȴ� (���� �޸� ũ�Ⱑ ���� ���� ���)
	if (FreeBlocks[0]->GetSize() >= AlignedSize)
	{
		return 0;
	}

	// ���� �޸� ũ�Ⱑ AlignedSize���� ���� ���� 
	// ���� ���� ���� ���� �޸� ũ�⸦ ���� Free ����� ã�� ��ȯ�Ѵ�.
	int32 FindIndex = 0;
	auto FindFreeBlock = std::find_if(FreeBlocks.begin(), FreeBlocks.end(), [&FindIndex, AlignedSize](const RHIPoolAllocationData* FreeBlock) {FindIndex++; return FreeBlock->GetSize() >= AlignedSize; });
	
	// �Ʒ� �� �� �˻� �� �ϳ��� ���� �� �� ����� �ϴ�.
	if (FindFreeBlock != FreeBlocks.end())
	{
		if (FindIndex < FreeBlocks.size())
		{
			return FindIndex;
		}
	}

	return -1;
}

Ideal::RHIPoolAllocationData* Ideal::RHIMemoryPool::AddToFreeBlocks(RHIPoolAllocationData* InFreeBlock)
{
	Check(InFreeBlock->IsFree());
	Check(IsAligned(InFreeBlock->GetSize(), PoolAlignment));
	Check(IsAligned(InFreeBlock->GetOffset(), PoolAlignment));

	RHIPoolAllocationData* FreeBlock = InFreeBlock;

	// ���� ��ϰ� ��ĥ �� �ִ��� �˻�
	if (FreeBlock->GetPrev()->IsFree() && !FreeBlock->GetPrev()->IsLocked())
	{
		RHIPoolAllocationData* PrevFree = FreeBlock->GetPrev();
		PrevFree->Merge(FreeBlock);
		RemoveFromFreeBlocks(PrevFree);
		ReleaseAllocationData(FreeBlock);

		FreeBlock = PrevFree;
	}

	// ���� ��ϰ� ��ĥ �� �ִ��� �˻�
	if (FreeBlock->GetNext()->IsFree() && !FreeBlock->GetNext()->IsLocked())
	{
		RHIPoolAllocationData* NextFree = FreeBlock->GetNext();
		FreeBlock->Merge(NextFree);
		RemoveFromFreeBlocks(NextFree);
		ReleaseAllocationData(NextFree);
	}

	// FreeBlocks�� �����ؾ� �ϴ� ��ġ�� ã�� �ִ´�.
	auto findIt = std::find_if(FreeBlocks.begin(), FreeBlocks.end(), [&](RHIPoolAllocationData* Left) { return Left->GetSize() > FreeBlock->GetSize(); });
	FreeBlocks.insert(findIt, FreeBlock);
	
	Validate();

	return FreeBlock;
}

void Ideal::RHIMemoryPool::RemoveFromFreeBlocks(RHIPoolAllocationData* InFreeBlock)
{
	for (int32 FreeBlockIndex = 0; FreeBlockIndex < FreeBlocks.size(); ++FreeBlockIndex)
	{
		if (FreeBlocks[FreeBlockIndex] == InFreeBlock)
		{
			FreeBlocks.erase(FreeBlocks.begin() + FreeBlockIndex);
			break;
		}
	}
}

void Ideal::RHIMemoryPool::ReleaseAllocationData(RHIPoolAllocationData* InData)
{
	InData->Reset();

	// ���� ĳ�û���� �̹� ����ϴٸ� �׳� ����� �ƴϸ� ĳ��
	if (AllocationDataPool.size() >= DesiredAllocationPoolSize)
	{
		delete InData;
	}
	else
	{
		AllocationDataPool.push_back(InData);
	}
}

Ideal::RHIPoolAllocationData* Ideal::RHIMemoryPool::GetNewAllocationData()
{
	//return(AllocationDataPool.size() > 0) ? AllocationDataPool.pop_back() : 

	if (AllocationDataPool.size() > 0)
	{
		RHIPoolAllocationData* back = AllocationDataPool.back();
		AllocationDataPool.pop_back();
		return back;
	}
	else
	{
		new RHIPoolAllocationData();
	}
}

void Ideal::RHIMemoryPool::Validate()
{
#ifdef RHI_POOL_ALLOCATOR_VALIDATE_LINK_LIST
	uint64 TotalFreeSize = 0;
	uint64 TotalAllocatedSize = 0;
	uint64 CurrentOffset = 0;

	// ���Ḯ��Ʈ�� �����Ѵ�.
	RHIPoolAllocationData* CurrentBlock = HeadBlock.GetNext();
	while (CurrentBlock != &HeadBlock)
	{
		Check(CurrentBlock == CurrentBlock->GetNext()->GetPrev());
		Check(CurrentBlock == CurrentBlock->GetPrev()->GetNext());

		uint32 AlignedSize = GetAlignedSize(CurrentBlock->GetSize(), PoolAlignment, CurrentBlock->GetAlignment());
		uint32 AlignedOffset = AlignDown(CurrentBlock->GetOffset(), PoolAlignment);

		Check(AlignedOffset == CurrentOffset);

		if (CurrentBlock->IsAllocated())
		{
			TotalAllocatedSize += AlignedSize;
		}
		else if (CurrentBlock->IsFree())
		{
			TotalFreeSize += AlignedSize;
		}
		else
		{
			Check(false);
		}

		CurrentOffset += AlignedSize;
		CurrentBlock = CurrentBlock->GetNext();
	}

	Check(TotalFreeSize + TotalAllocatedSize == PoolSize);
	Check(TotalFreeSize == FreeSize);
	Check(TotalAllocatedSize == GetUsedSize());

	// FreeBlock ����
	TotalFreeSize = 0;
	for (int32 FreeBlockIndex = 0; FreeBlockIndex < FreeBlocks.size(); ++FreeBlockIndex)
	{
		RHIPoolAllocationData* FreeBlock = FreeBlocks[FreeBlockIndex];
		Check(!FreeBlock->GetPrev()->IsFree() || FreeBlock->GetPrev()->IsLocked());
		Check(!FreeBlock->GetNext()->IsFree() || FreeBlock->GetNext()->IsLocked());
		Check(FreeBlockIndex == (FreeBlocks.size() - 1));
		//Check(FreeBlock->GetSize() <= FreeBlocks[FreeBlockIndex + 1]->GetSize());
		Check(IsAligned(FreeBlock->GetOffset(), PoolAlignment));
		TotalFreeSize += FreeBlock->GetSize();
	}
	Check(TotalFreeSize == FreeSize);
#endif
}

/// <summary>
///  D3D12MemoryPool
/// </summary>
/// <param name="Device"></param>

Ideal::D3D12MemoryPool::D3D12MemoryPool(
	ID3D12Device* InDevice
	, uint32 InPoolIndex
	, uint64 InPoolSize
	, uint32 InPoolAlignment
	, EResourceAllocationStrategy InAllocationStrategy
	, D3D12ResourceInitConfig InInitConfig
	) : RHIMemoryPool(InPoolIndex, InPoolSize, InPoolAlignment)
	, D3D12DeviceChild(InDevice)
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