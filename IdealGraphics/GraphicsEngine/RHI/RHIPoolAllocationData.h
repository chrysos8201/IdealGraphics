#pragma once
#include "Core\Core.h"

namespace Ideal
{
	struct RHIPoolAllocationData
	{
	public:
		void Reset();

		void InitAsHead(int16 InPoolIndex);
		void InitAsAllocated(uint32 InSize, uint32 InPoolAlignment, uint32 InAllocationAlignment, RHIPoolAllocationData* InFree);

		RHIPoolAllocationData* GetNext() const { return NextAllocation; }
		RHIPoolAllocationData* GetPrev() const { return PreviousAllocation; }
		uint32 GetSize() const { return Size; }
		uint32 GetOffset() const { return Offset; }
		uint32 GetAlignment() const { return Alignment; }
		int16 GetPoolIndex() const { return PoolIndex; }

		bool IsHead() const { return GetAllocationType() == EAllocationType::Head; }
		bool IsFree() const { return GetAllocationType() == EAllocationType::Free; }
		bool IsAllocated() const { return GetAllocationType() == EAllocationType::Allocated; }

		bool IsLocked() const { return (Locked == 1); }
		void UnLock() { Check(Locked == 1); Locked = 0; }

	private:

		friend class RHIMemoryPool;
		friend class RHIPoolAllocator;

		void Merge(RHIPoolAllocationData* InOther);

		void RemoveFromLinkedList();
		void AddBefore(RHIPoolAllocationData* InOther);

		enum class EAllocationType : uint8
		{
			UnKnown,
			Free,
			Allocated,
			Head,
		};
		EAllocationType GetAllocationType() const { return (EAllocationType)Type; }
		void SetAllocationType(EAllocationType InType) { Type = (uint32)InType; }

		uint32 Size;
		uint32 Alignment;
		uint32 Offset;
		uint16 PoolIndex;
		uint32 Type;
		uint32 Locked;
		uint32 Unused;

		// memory pool���Լ� �����޾ƾ� �Ѵ�.
		RHIPoolAllocationData* PreviousAllocation;
		RHIPoolAllocationData* NextAllocation;
	};
}