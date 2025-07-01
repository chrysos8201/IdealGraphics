#include "RHIPoolAllocationData.h"
#include "Core\Core.h"
#include "RHIMemoryPool.h"
#include "D3D12/D3D12Util.h"

void Ideal::RHIPoolAllocationData::Reset()
{
	Size = 0;
	Alignment = 0;
	SetAllocationType(EAllocationType::UnKnown);
	PoolIndex = -1;
	Offset = 0;
	Locked = false;

	PreviousAllocation = nullptr;
	NextAllocation = nullptr;
}

void Ideal::RHIPoolAllocationData::InitAsHead(int16 InPoolIndex)
{
	Reset();
	
	SetAllocationType(EAllocationType::Head);
	NextAllocation = this;
	PreviousAllocation = this;
	PoolIndex = InPoolIndex;
}

void Ideal::RHIPoolAllocationData::InitAsAllocated(uint32 InSize, uint32 InPoolAlignment, uint32 InAllocationAlignment, RHIPoolAllocationData* InFree)
{
	Check(InFree->IsFree());
	
	Reset();

	Size = InSize;
	Alignment = InAllocationAlignment;
	SetAllocationType(EAllocationType::Allocated);
	Offset = RHIMemoryPool::GetAlignedOffset(InFree->Offset, InPoolAlignment, InAllocationAlignment);
	PoolIndex = InFree->PoolIndex;
	Locked = true;

	uint32 AlignedSize = RHIMemoryPool::GetAlignedSize(InSize, InPoolAlignment, InAllocationAlignment);
	InFree->Size -= AlignedSize;
	InFree->Offset += AlignedSize;
	InFree->AddBefore(this);
}

void Ideal::RHIPoolAllocationData::InitAsFree(int16 InPoolIndex, uint32 InSize, uint32 InAlignment, uint32 InOffset)
{
	Reset();

	Size = InSize;
	Alignment = InAlignment;
	SetAllocationType(EAllocationType::Free);
	Offset = InOffset;
	PoolIndex = InPoolIndex;
}

void Ideal::RHIPoolAllocationData::MoveFrom(RHIPoolAllocationData& InAllocated, bool InLocked)
{
	/*1. FD3D12ResourceLocation은 이미 할당된 메모리 블록(노드)을 소유하고 있다. 이 노드는 현재 사용 중이거나 사용이 완료되어 회수될 준비이다.
	2. 노드를 즉시 해제하는 대신, PoolAllocator는 이 노드의 속성을 “중간 노드” 또는 “새로운 빈 노드”로 MoveFrom함수를 사용하여 이동시킨다.
	3. 이렇게 하는 이유:
		a. FD3D12ResourceLocation이 더 이상 해당 메모리 블록의 유효한 데이터를 소유하지 않도록 소유권을 분리한다. InAllocated(FD3D12ResourceLocation이 보유했던 노드)는 MoveFrom 호출 후 Reset() 되어 속성이 지워진다.
		b. 실제 GPU 메모리 회수 작업(DeallocateInternal)은 즉시 발생하는 것이 아니라, FrameFencedOperations라는 큐에 추가되어 해당 프레임의 모든 GPU 명령이 완료된 후 지연되어 처리된다.
	→ MoveFrom을 통해 노드의 논리적 상태를 변경하고 임시 저장소로 옮긴 후, 실제 물리적 메모리 해제는 나중에 수행된다.*/
	Check(InAllocated.IsAllocated());
	Reset();

	Size = InAllocated.Size;
	Alignment = InAllocated.Alignment;
	Type = InAllocated.Type;
	Offset = InAllocated.Offset;
	PoolIndex = InAllocated.PoolIndex;
	Locked = InLocked;

	if (InAllocated.PreviousAllocation)
	{
		InAllocated.PreviousAllocation->NextAllocation = this;
		InAllocated.NextAllocation->PreviousAllocation = this;
		PreviousAllocation = InAllocated.PreviousAllocation;
		NextAllocation = InAllocated.NextAllocation;
	}

	InAllocated.Reset();
}

void Ideal::RHIPoolAllocationData::MarkFree(uint32 InPoolAlignment, uint32 InAllocationAlignment)
{
	Check(IsAllocated());

	SetAllocationType(EAllocationType::Free);
	Size = RHIMemoryPool::GetAlignedSize(Size, InPoolAlignment, InAllocationAlignment);
	Offset = AlignDown(Offset, InPoolAlignment);
	Alignment = InPoolAlignment;
}

void Ideal::RHIPoolAllocationData::Merge(RHIPoolAllocationData* InOther)
{
	Check(IsFree() && InOther->IsFree());
	Check((Offset + Size) == InOther->Offset);
	Check(PoolIndex == InOther->GetPoolIndex());

	Size += InOther->Size;
	InOther->RemoveFromLinkedList();
	InOther->Reset();
}

void Ideal::RHIPoolAllocationData::RemoveFromLinkedList()
{
	Check(IsFree());
	PreviousAllocation->NextAllocation = NextAllocation;
	NextAllocation->PreviousAllocation = PreviousAllocation;
}

void Ideal::RHIPoolAllocationData::AddBefore(RHIPoolAllocationData* InOther)
{
	PreviousAllocation->NextAllocation = InOther;
	InOther->PreviousAllocation = PreviousAllocation;

	PreviousAllocation = InOther;
	InOther->NextAllocation = this;
}

void Ideal::RHIPoolAllocationData::AddAfter(RHIPoolAllocationData* InOther)
{
	NextAllocation->PreviousAllocation = InOther;
	InOther->NextAllocation = NextAllocation;

	NextAllocation = InOther;
	InOther->PreviousAllocation = this;
}
