#include "D3D12DescriptorManager.h"

Ideal::D3D12DescriptorManager::D3D12DescriptorManager()
{

}

Ideal::D3D12DescriptorManager::~D3D12DescriptorManager()
{

}

void Ideal::D3D12DescriptorManager::Create(ComPtr<ID3D12Device> Device, uint32 NumDescriptorPool, uint32 MaxCountDescriptorPool, uint32 NumFixedDescriptorHeap, uint32 MaxCountFixedDescriptorHeap)
{
	m_numDescriptorPool = NumDescriptorPool;
	m_numFixedDescriptorPool = NumFixedDescriptorHeap;
	
	m_countDescriptorPool.resize(NumDescriptorPool);
	m_maxCountDescriptorPool = MaxCountDescriptorPool;

	m_countFixedDescriptorPool.resize(NumFixedDescriptorHeap);
	m_maxCountFixedDescriptorPool = MaxCountFixedDescriptorHeap;

	m_indexCreator.resize(NumFixedDescriptorHeap);
	for (auto& indexCreator : m_indexCreator)
	{
		indexCreator.Init(MaxCountFixedDescriptorHeap);
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	uint32 NumDescriptors = MaxCountDescriptorPool * NumDescriptorPool + MaxCountFixedDescriptorHeap * NumFixedDescriptorHeap;
	heapDesc.NumDescriptors = NumDescriptors;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	Check(
		Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)),
		L"Failed to create Descriptor Manager"
	);
	m_descriptorSize = Device->GetDescriptorHandleIncrementSize(heapDesc.Type);

	m_descriptorHeap->SetName(L"D3D12DescriptorManager");
}

void Ideal::D3D12DescriptorManager::ResetPool(uint32 PoolIndex)
{
	m_countDescriptorPool[PoolIndex] = 0;
}

void Ideal::D3D12DescriptorManager::ResetFixed(uint32 FixedDescriptorHeapIndex)
{
	m_countFixedDescriptorPool[FixedDescriptorHeapIndex] = 0;
}

Ideal::D3D12DescriptorHandle Ideal::D3D12DescriptorManager::Allocate(uint32 DescriptorPoolIndex)
{
	uint32 maxCount = m_maxCountDescriptorPool;
	uint32 count = m_countDescriptorPool[DescriptorPoolIndex];
	if (count >= maxCount)
	{
		MyMessageBoxW(L"Can't Allocate Descriptor In DescriptorManager");
		__debugbreak();
		return Ideal::D3D12DescriptorHandle();
	}

	Ideal::D3D12DescriptorHandle ret = Ideal::D3D12DescriptorHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_descriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);

	ret += DescriptorPoolIndex * maxCount * m_descriptorSize + count * m_descriptorSize;
	m_countDescriptorPool[DescriptorPoolIndex]++;

	return ret;
}

Ideal::D3D12DescriptorHandle Ideal::D3D12DescriptorManager::AllocateFixed(uint32 FixedDescriptorHeapIndex)
{
	uint32 maxCount = m_maxCountFixedDescriptorPool;
	uint32 count = m_countFixedDescriptorPool[FixedDescriptorHeapIndex];
	if (count >= maxCount)
	{
		MyMessageBoxW(L"Can't Allocate Descriptor In DescriptorManager");
		__debugbreak();
		return Ideal::D3D12DescriptorHandle();
	}
	int32 index = m_indexCreator[FixedDescriptorHeapIndex].Allocate();

	Ideal::D3D12DescriptorHandle ret = Ideal::D3D12DescriptorHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
		shared_from_this(),
		index,	// 빈 index
		FixedDescriptorHeapIndex	// fixed heap의 index값 
	);

	// 2024.07.06 아래에서 index가 아닌 count를 넘겨주어서 GPU메모리를 잘못 접근해서 터졌던 것을 고쳤다....

	ret += m_numDescriptorPool * m_maxCountDescriptorPool * m_descriptorSize;
	ret += FixedDescriptorHeapIndex * m_maxCountFixedDescriptorPool * m_descriptorSize + index * m_descriptorSize;
	m_countFixedDescriptorPool[FixedDescriptorHeapIndex]++;

	

	return ret;
}

void Ideal::D3D12DescriptorManager::Free(int32 FixedDescriptorHeapIndex, int32 AllocatedIndex)
{
	if (AllocatedIndex < 0)
	{
		MyMessageBoxW(L"Can't Free Descriptor In DescriptorManager");
		__debugbreak();
		return;
	}

	if (FixedDescriptorHeapIndex < 0)
	{
		MyMessageBoxW(L"Can't Free Descriptor In DescriptorManager");
		__debugbreak();
		return;
	}

	m_countFixedDescriptorPool[FixedDescriptorHeapIndex]--;
	m_indexCreator[FixedDescriptorHeapIndex].Free(AllocatedIndex);
}
