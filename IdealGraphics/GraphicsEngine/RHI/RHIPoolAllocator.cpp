#include "RHIPoolAllocator.h"
#include "RHIPoolAllocationData.h"
#include "Core\Core.h"

bool Ideal::RHIPoolAllocator::TryAllocateInternal(uint32 InSizeInBytes, RHIPoolAllocationData& InAllocationData)
{
	// TODO : 현재 필요한 리소스의 속성과 맞는 Pool을 가져와야함. -> 이미 Pool ALlocator에서 한번 거를 수 있음.
	// 만약 없으면 Pool을 새로 만들고 거기서 할당

	Check((PoolAllocationOrder.size() == Pools.size()));

	for (uint32 PoolIndex : PoolAllocationOrder)
	{
		RHIMemoryPool* Pool = Pools[PoolIndex];
		// 해당 Pool에서 할당할 수 있는지 확인
		if (Pool != nullptr)
		{
			// TODO : 여기서 Pool Allocator 시도 하고 성공하면 true 반환.
			// 시도하면서 내부에서 AllocationData를 채워야 함.
			// 성공하면 여기서 함수를 끝낸다.
		}
	}

	// 여기서 조각 모음하며 해제됐던 Pool이면 해당 Pool이 nullptr일 것이므로 다시 해당 PoolIndex로 만든다.
	int16 NewPoolIndex = -1;
	for (int32 PoolIndex = 0; PoolIndex < Pools.size(); PoolIndex++)
	{
		if (Pools[PoolIndex] == nullptr)
		{
			NewPoolIndex = PoolIndex;
			break;
		}
	}

	if (NewPoolIndex >= 0)
	{
		// 비어있던 Pool 채우기
		// TODO : CreateNewPool
		//Pools[NewPoolIndex] = 
	}
	else
	{
		// 마지막에 채움
		NewPoolIndex = Pools.size();
		// TODO : 새로 만들고 Pools에 넣을 것
		PoolAllocationOrder.push_back(NewPoolIndex);
	}

	// TODO : 새로 만든 Pools[NewPoolIndex]에서 AllocationPoolData에 할당한다.
	// 할당 여부에 따라 bool 반환
	return false;
}

Ideal::RHIMemoryPool* Ideal::RHIPoolAllocator::CreateNewPool(uint32 InPoolIndex)
{
	//RHIMemoryPool* NewPool = new RHIMemoryPool();
	// 2025.06.17 :: 01.44 여기 하는 중
}
