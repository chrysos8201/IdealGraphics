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
	/*1. FD3D12ResourceLocation�� �̹� �Ҵ�� �޸� ���(���)�� �����ϰ� �ִ�. �� ���� ���� ��� ���̰ų� ����� �Ϸ�Ǿ� ȸ���� �غ��̴�.
	2. ��带 ��� �����ϴ� ���, PoolAllocator�� �� ����� �Ӽ��� ���߰� ��塱 �Ǵ� �����ο� �� ��塱�� MoveFrom�Լ��� ����Ͽ� �̵���Ų��.
	3. �̷��� �ϴ� ����:
		a. FD3D12ResourceLocation�� �� �̻� �ش� �޸� ����� ��ȿ�� �����͸� �������� �ʵ��� �������� �и��Ѵ�. InAllocated(FD3D12ResourceLocation�� �����ߴ� ���)�� MoveFrom ȣ�� �� Reset() �Ǿ� �Ӽ��� ��������.
		b. ���� GPU �޸� ȸ�� �۾�(DeallocateInternal)�� ��� �߻��ϴ� ���� �ƴ϶�, FrameFencedOperations��� ť�� �߰��Ǿ� �ش� �������� ��� GPU ����� �Ϸ�� �� �����Ǿ� ó���ȴ�.
	�� MoveFrom�� ���� ����� ���� ���¸� �����ϰ� �ӽ� ����ҷ� �ű� ��, ���� ������ �޸� ������ ���߿� ����ȴ�.*/
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
