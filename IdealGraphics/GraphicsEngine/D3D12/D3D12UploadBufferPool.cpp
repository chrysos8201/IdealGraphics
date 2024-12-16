#include "D3D12UploadBufferPool.h"

Ideal::D3D12UploadBufferPool::D3D12UploadBufferPool()
	: m_resource(nullptr),
	m_offset(0),
	m_maxInstanceNum(0),
	m_systemMemAddr(nullptr),
	m_gpuVirtualAddress(0),
	m_allocatedInstanceNum(0)
{

}

Ideal::D3D12UploadBufferPool::~D3D12UploadBufferPool()
{

}

void Ideal::D3D12UploadBufferPool::Init(ID3D12Device* Device, uint32 Offset, uint32 MaxInstanceNum)
{
	m_maxInstanceNum = MaxInstanceNum;
	m_offset = Offset;
	uint32 byteWidth = m_offset * m_maxInstanceNum;

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
		))
		, L"Failed to Create Upload Buffer Heap!"
	);

	// Get Address
	CD3DX12_RANGE writeRange(0, 0);
	m_resource->Map(0, &writeRange, reinterpret_cast<void**>(&m_systemMemAddr));

	/// Create Container
	m_uploadBufferContainers.resize(MaxInstanceNum);
	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();
	// handle
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = m_gpuVirtualAddress;
	uint8* systemMemoryAddress = m_systemMemAddr;
	for (uint32 i = 0; i < m_maxInstanceNum; ++i)
	{
		m_uploadBufferContainers[i] = std::make_shared<Ideal::UploadBufferContainer>();
		m_uploadBufferContainers[i]->GpuVirtualAddress = gpuVirtualAddress;
		m_uploadBufferContainers[i]->SystemMemoryAddress = systemMemoryAddress;

		// Offset
		gpuVirtualAddress += Offset;
		systemMemoryAddress += Offset;
	}
}

std::shared_ptr<Ideal::UploadBufferContainer> Ideal::D3D12UploadBufferPool::Allocate()
{
	std::shared_ptr<UploadBufferContainer> ret = nullptr;
	if (m_allocatedInstanceNum >= m_maxInstanceNum)
	{
		__debugbreak();
		return ret;
	}

	ret = m_uploadBufferContainers[m_allocatedInstanceNum];
	m_allocatedInstanceNum++;
	return ret;
}

void Ideal::D3D12UploadBufferPool::Reset()
{
	m_allocatedInstanceNum = 0;
}
