#include "IndexCreator.h"

IndexCreator::IndexCreator()
	: m_allocatedCount(0),
	m_maxNum(0)
{
	
}

IndexCreator::~IndexCreator()
{
	if (m_allocatedCount)
	{
		// 아직 할당하고 있는 수가 있는데 초기화시켜버림
		__debugbreak();
	}

	if (m_table)
	{
		delete[] m_table;
		m_table = nullptr;
	}
}

void IndexCreator::Init(uint32 MaxNum)
{
	m_maxNum = MaxNum;
	m_table = new uint32[MaxNum];
	::memset(m_table, 0, sizeof(uint32) * MaxNum);	
	for (uint32 i = 0; i < MaxNum; ++i)
	{
		m_table[i] = i;
	}
}

int32 IndexCreator::Allocate()
{
	if (m_allocatedCount >= m_maxNum)
	{
		return -1;
	}
	return m_table[m_allocatedCount++];
}

void IndexCreator::Free(uint32 Index)
{
	if (!m_allocatedCount)
	{
		// 더 이상 해제할게 없음
		//__debugbreak();
		return;
	}
	m_allocatedCount--;
	m_table[m_allocatedCount] = Index;
}
