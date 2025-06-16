#include "RHIPoolAllocator.h"
#include "RHIPoolAllocationData.h"
#include "Core\Core.h"

bool Ideal::RHIPoolAllocator::TryAllocateInternal(uint32 InSizeInBytes, RHIPoolAllocationData& InAllocationData)
{
	// TODO : ���� �ʿ��� ���ҽ��� �Ӽ��� �´� Pool�� �����;���. -> �̹� Pool ALlocator���� �ѹ� �Ÿ� �� ����.
	// ���� ������ Pool�� ���� ����� �ű⼭ �Ҵ�

	Check((PoolAllocationOrder.size() == Pools.size()));

	for (uint32 PoolIndex : PoolAllocationOrder)
	{
		RHIMemoryPool* Pool = Pools[PoolIndex];
		// �ش� Pool���� �Ҵ��� �� �ִ��� Ȯ��
		if (Pool != nullptr)
		{
			// TODO : ���⼭ Pool Allocator �õ� �ϰ� �����ϸ� true ��ȯ.
			// �õ��ϸ鼭 ���ο��� AllocationData�� ä���� ��.
			// �����ϸ� ���⼭ �Լ��� ������.
		}
	}

	// ���⼭ ���� �����ϸ� �����ƴ� Pool�̸� �ش� Pool�� nullptr�� ���̹Ƿ� �ٽ� �ش� PoolIndex�� �����.
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
		// ����ִ� Pool ä���
		// TODO : CreateNewPool
		//Pools[NewPoolIndex] = 
	}
	else
	{
		// �������� ä��
		NewPoolIndex = Pools.size();
		// TODO : ���� ����� Pools�� ���� ��
		PoolAllocationOrder.push_back(NewPoolIndex);
	}

	// TODO : ���� ���� Pools[NewPoolIndex]���� AllocationPoolData�� �Ҵ��Ѵ�.
	// �Ҵ� ���ο� ���� bool ��ȯ
	return false;
}

Ideal::RHIMemoryPool* Ideal::RHIPoolAllocator::CreateNewPool(uint32 InPoolIndex)
{
	//RHIMemoryPool* NewPool = new RHIMemoryPool();
	// 2025.06.17 :: 01.44 ���� �ϴ� ��
}
