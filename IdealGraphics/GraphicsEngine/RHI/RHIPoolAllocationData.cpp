#include "RHIPoolAllocationData.h"

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
