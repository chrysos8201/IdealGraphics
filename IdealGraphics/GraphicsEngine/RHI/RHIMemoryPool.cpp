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
	return AlignArbitrary(Size, InPoolAlignment);	// 정렬
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

		// 일단 블록을 꺼내와서 제거한다. 만약 꺼내온 블록에서 남는 공간이 생기면 나중에 다시 넣고 정렬할 것이다.
		FreeBlocks.erase(FreeBlocks.begin() + FreeBlockIndex);

		// 꺼내온 블록을 AllocationData에 넣고 남은 블록은 FreeBlock에 계속 남아있다.
		AllocationData.InitAsAllocated(InSizeInBytes, PoolAlignment, InAllocationAlignment, FreeBlock);
		Check((AllocationData.GetOffset() % InAllocationAlignment) == 0);
		
		// 정보 업데이트
		Check(FreeSize >= AlignedSize);
		FreeSize -= AlignedSize;
		AlignmentWaste += (AlignedSize - InSizeInBytes);
		AllocatedBlocks++;
		

		// 만약 남는 블록의 사이즈가 0인 경우 새로 할당받은 블록이랑 딱 맞는다는 뜻
		if (FreeBlock->GetSize() == 0)
		{
			// 연결된 링크드 리스트에서 freeblock 앞과 뒤를 서로 연결해준다.
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
	
	// MemoryPool에서 사용할 수 있는 크기를 넘어서면 일찍 종료한다.
	if (FreeSize < AlignedSize)
	{
		return -1;
	}

	// 무조건 사이즈로 정렬한다.

	// FreeBlocks 에서 찾는다.
	// 첫 번째 블록의 크기가 요구 사항을 만족하면
	// 항상 첫 번째 노드가 반환된다 (비디오 메모리 크기가 가장 작은 노드)
	if (FreeBlocks[0]->GetSize() >= AlignedSize)
	{
		return 0;
	}

	// 비디오 메모리 크기가 AlignedSize보다 작지 않은 
	// 가장 작은 작은 비디오 메모리 크기를 가진 Free 블록을 찾아 반환한다.
	int32 FindIndex = 0;
	auto FindFreeBlock = std::find_if(FreeBlocks.begin(), FreeBlocks.end(), [&FindIndex, AlignedSize](const RHIPoolAllocationData* FreeBlock) {FindIndex++; return FreeBlock->GetSize() >= AlignedSize; });
	
	// 아래 두 개 검사 중 하나는 빼도 될 것 같기는 하다.
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

	// 이전 블록과 합칠 수 있는지 검사
	if (FreeBlock->GetPrev()->IsFree() && !FreeBlock->GetPrev()->IsLocked())
	{
		RHIPoolAllocationData* PrevFree = FreeBlock->GetPrev();
		PrevFree->Merge(FreeBlock);
		RemoveFromFreeBlocks(PrevFree);
		ReleaseAllocationData(FreeBlock);

		FreeBlock = PrevFree;
	}

	// 다음 블록과 합칠 수 있는지 검사
	if (FreeBlock->GetNext()->IsFree() && !FreeBlock->GetNext()->IsLocked())
	{
		RHIPoolAllocationData* NextFree = FreeBlock->GetNext();
		FreeBlock->Merge(NextFree);
		RemoveFromFreeBlocks(NextFree);
		ReleaseAllocationData(NextFree);
	}

	// FreeBlocks를 삽입해야 하는 위치를 찾고 넣는다.
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

	// 만약 캐시사이즈가 이미 충분하다면 그냥 지우고 아니면 캐시
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

	// 연결리스트를 검증한다.
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

	// FreeBlock 검증
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

	// ID3D12Heap을 만드는 경우
	if (AllocationStrategy == EResourceAllocationStrategy::PlacedResource)
	{
		// GPU Resource 메모리 정렬이 4KB 이거나 64KB이어야 한다
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
	// ID3D12Resource를 만드는 경우
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