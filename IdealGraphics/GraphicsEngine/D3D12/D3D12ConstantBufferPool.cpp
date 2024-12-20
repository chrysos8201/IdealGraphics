#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include <d3dx12.h>
using namespace Ideal;

D3D12ConstantBufferPool::D3D12ConstantBufferPool()
	: m_resource(nullptr),
	m_cbvHeap(nullptr),
	m_systemMemoryAddress(nullptr),
	m_maxCBVNum(0),
	m_sizePerCBV(0),
	m_allocatedCBVNum(0)
{

}

D3D12ConstantBufferPool::~D3D12ConstantBufferPool()
{

}

void D3D12ConstantBufferPool::Init(ID3D12Device* Device, uint32 SizePerCBV, uint32 MaxCBVNum)
{
	m_maxCBVNum = MaxCBVNum;
	m_sizePerCBV = SizePerCBV;
	uint32 byteWidth = m_sizePerCBV * m_maxCBVNum;

	// Create Buffer
	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteWidth);
	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf()
		)));
	
	// Get Address
	CD3DX12_RANGE writeRange(0, 0);
	m_resource->Map(0, &writeRange, reinterpret_cast<void**>(&m_systemMemoryAddress));

	// Create Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = m_maxCBVNum;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Check(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

	m_cbContainer.resize(m_maxCBVNum);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_sizePerCBV;

	uint8* systemMemPtr = m_systemMemoryAddress;
	CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

	uint32 DescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 개수 만큼 미리 다 만들어둔다.
	for (uint32 i = 0; i < m_maxCBVNum; ++i)
	{
		Device->CreateConstantBufferView(&cbvDesc, heapHandle);
		
		m_cbContainer[i] = std::make_shared<ConstantBufferContainer>();
		m_cbContainer[i]->CpuHandle = heapHandle;
		m_cbContainer[i]->GpuMemoryAddress = cbvDesc.BufferLocation;
		m_cbContainer[i]->SystemMemoryAddress = systemMemPtr;
		
		heapHandle.Offset(1, DescriptorSize);
		cbvDesc.BufferLocation += m_sizePerCBV;
		systemMemPtr += m_sizePerCBV;
	}
}

std::shared_ptr<ConstantBufferContainer> D3D12ConstantBufferPool::Allocate()
{
	std::shared_ptr<ConstantBufferContainer> ret = nullptr;
	// 만약 현재 할당된 개수가 최대 개수 or 넘어가면 nullptr 반환
	if (m_allocatedCBVNum >= m_maxCBVNum)
	{
		return ret;
	}

	ret = m_cbContainer[m_allocatedCBVNum];
	m_allocatedCBVNum++;
	return ret;
}

void D3D12ConstantBufferPool::Reset()
{
	m_allocatedCBVNum = 0;
}
