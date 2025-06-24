#include "RHIPoolAllocationData.h"
#include "Core\Core.h"
#include "RHIMemoryPool.h"

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
