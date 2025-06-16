#pragma once
#include "Core\Types.h"
#include <vector>

namespace Ideal { struct RHIPoolAllocationData; }

namespace Ideal { class RHIMemoryPool; }

namespace Ideal
{
	class RHIPoolAllocator
	{
	public:
		
	protected:
		bool TryAllocateInternal(uint32 InSizeInBytes, RHIPoolAllocationData& InAllocationData);
		RHIMemoryPool* CreateNewPool(uint32 InPoolIndex);

		std::vector<RHIMemoryPool*> Pools;

		// PoolÀ» Á¤·Ä
		std::vector<uint32> PoolAllocationOrder;
	};
}

