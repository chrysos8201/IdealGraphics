#pragma once
#include "Core\Types.h"

namespace Ideal
{
	struct RHIPoolAllocationData
	{
	public:
		void Reset();

		void InitAsHead(int16 InPoolIndex);

		RHIPoolAllocationData* GetNext() const { return NextAllocation; }
		RHIPoolAllocationData* GetPrev() const { return PreviousAllocation; }

	private:

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
		uint32 PoolIndex;
		uint32 Type;
		uint32 Locked;
		uint32 Unused;

		// memory pool에게서 관리받아야 한다.
		RHIPoolAllocationData* PreviousAllocation;
		RHIPoolAllocationData* NextAllocation;
	};
}