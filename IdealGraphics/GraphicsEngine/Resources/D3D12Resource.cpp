#include "GraphicsEngine/Resources/D3D12Resource.h"

using namespace Ideal;

///////////////////<D3D12Resource>//////////////////////
D3D12Resource::D3D12Resource()
{

}

D3D12Resource::~D3D12Resource()
{

}

ID3D12Resource* D3D12Resource::GetResource() const
{
	return m_resource.Get();
}

///////////////////<UploadBuffer>//////////////////////

D3D12UploadBuffer::D3D12UploadBuffer()
	: m_bufferSize(0)
{

}

D3D12UploadBuffer::~D3D12UploadBuffer()
{

}

void Ideal::D3D12UploadBuffer::Create(ID3D12Device* Device, uint32 BufferSize)
{
	// 버퍼 사이즈
	m_bufferSize = BufferSize;

	// 업로드 용으로 버퍼를 만든다
	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf())
	));
}

void* D3D12UploadBuffer::Map()
{
	void* beginData;
	// 두번째 인자에 nullptr을 넣으면 전체 하위 리소스를 읽을 수 있음을 나타낸다.
	Check(m_resource->Map(0, nullptr, &beginData));
	return beginData;
}

void D3D12UploadBuffer::UnMap()
{
	m_resource->Unmap(0, nullptr);
}

///////////////////<GPUBuffer>//////////////////////

D3D12GPUBuffer::D3D12GPUBuffer()
	: m_bufferSize(0),
	m_elementSize(0),
	m_elementCount(0)
{

}

D3D12GPUBuffer::~D3D12GPUBuffer()
{

}

void D3D12GPUBuffer::CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount)
{
	m_bufferSize = ElementSize * ElementCount;
	m_elementSize = ElementSize;
	m_elementCount = ElementCount;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf())
	));
}

uint32 D3D12GPUBuffer::GetBufferSize() const
{
	return m_bufferSize;
}

uint32 D3D12GPUBuffer::GetElementCount() const
{
	return m_elementCount;
}

uint32 D3D12GPUBuffer::GetElemnetSize() const
{
	return m_elementSize;
}

///////////////////<VertexBuffer>//////////////////////

D3D12VertexBuffer::D3D12VertexBuffer()
{

}

D3D12VertexBuffer::~D3D12VertexBuffer()
{

}

D3D12_VERTEX_BUFFER_VIEW D3D12VertexBuffer::GetView() const
{
	return m_vertexBufferView;
}

void D3D12VertexBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount);

	// 데이터를 복사한다
	CmdList->CopyBufferRegion(m_resource.Get(), 0, UploadBuffer.GetResource(), 0, m_bufferSize);

	CD3DX12_RESOURCE_BARRIER resourceBarrier
		= CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);

	CmdList->ResourceBarrier(1, &resourceBarrier);

	m_vertexBufferView.BufferLocation = m_resource->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = m_bufferSize;
	m_vertexBufferView.StrideInBytes = m_elementSize;
}

///////////////////<IndexBuffer>//////////////////////

D3D12IndexBuffer::D3D12IndexBuffer()
{

}

D3D12IndexBuffer::~D3D12IndexBuffer()
{

}

void D3D12IndexBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount);

	CmdList->CopyBufferRegion(m_resource.Get(), 0, UploadBuffer.GetResource(), 0, m_bufferSize);

	CD3DX12_RESOURCE_BARRIER resourceBarrier
		= CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
		);
	CmdList->ResourceBarrier(1, &resourceBarrier);

	m_indexBufferView.BufferLocation = m_resource->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = m_bufferSize;
}

D3D12_INDEX_BUFFER_VIEW D3D12IndexBuffer::GetView() const
{
	return m_indexBufferView;
}
