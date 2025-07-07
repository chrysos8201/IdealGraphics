#pragma once
#include "Core\Core.h"
//#include "D3D12\D3D12Common.h"

using namespace Ideal;
namespace Ideal { class D3D12ResourceLocation; }
namespace Ideal { namespace Visualization { class RHIProfiler; } }
namespace Ideal
{
	struct RHIPoolAllocationData
	{
	public:
		friend class Visualization::RHIProfiler;
	public:
		void Reset();

		void InitAsHead(int16 InPoolIndex);
		void InitAsAllocated(uint32 InSize, uint32 InPoolAlignment, uint32 InAllocationAlignment, RHIPoolAllocationData* InFree);
		void InitAsFree(int16 InPoolIndex, uint32 InSize, uint32 InAlignment, uint32 InOffset);
		void MoveFrom(RHIPoolAllocationData& InAllocated, bool InLocked);
		void MarkFree(uint32 InPoolAlignment, uint32 InAllocationAlignment);

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

		void SetOwner(RHIPoolResource* InOwner) { Owner = InOwner; }
		RHIPoolResource* GetOwner() const { return Owner; }

	private:

		friend class RHIMemoryPool;
		friend class RHIPoolAllocator;

		void Merge(RHIPoolAllocationData* InOther);

		void RemoveFromLinkedList();
		void AddBefore(RHIPoolAllocationData* InOther);
		void AddAfter(RHIPoolAllocationData* InOther);

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

		// memory pool에게서 관리받아야 한다.
		RHIPoolAllocationData* PreviousAllocation;
		RHIPoolAllocationData* NextAllocation;

		RHIPoolResource* Owner;	// D3D12ResourceLocation으로 상속
	};
}