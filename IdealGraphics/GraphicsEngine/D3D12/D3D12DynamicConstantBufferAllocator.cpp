#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include <d3d12.h>

Ideal::D3D12DynamicConstantBufferAllocator::D3D12DynamicConstantBufferAllocator()
	: m_maxCBVNumPerPool(0)
{

}

Ideal::D3D12DynamicConstantBufferAllocator::~D3D12DynamicConstantBufferAllocator()
{

}

void Ideal::D3D12DynamicConstantBufferAllocator::Init(const uint32& MaxCBVNum)
{
	m_maxCBVNumPerPool = MaxCBVNum;
}

std::shared_ptr<Ideal::ConstantBufferContainer> Ideal::D3D12DynamicConstantBufferAllocator::Allocate(ComPtr<ID3D12Device> Device, const uint32& Size)
{
	uint32 alignedSize = static_cast<uint32>(AlignConstantBufferSize(Size));
	auto it = m_cbPools.find(alignedSize);

	if (it == m_cbPools.end())
	{
		std::shared_ptr<Ideal::D3D12ConstantBufferPool> newPool = std::make_shared<Ideal::D3D12ConstantBufferPool>();
		newPool->Init(Device.Get(), alignedSize, m_maxCBVNumPerPool);
		m_cbPools[alignedSize] = newPool;
		it = m_cbPools.find(alignedSize);
	}
	return it->second->Allocate();
}

void Ideal::D3D12DynamicConstantBufferAllocator::Reset()
{
	for (auto pool : m_cbPools)
	{
		pool.second->Reset();
	}
}
