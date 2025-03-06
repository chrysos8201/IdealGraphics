#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12UAV.h"
using namespace Ideal;

//------------------------Resource------------------------//

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

void D3D12Resource::Release()
{
	//m_resource->Release();
	//m_resource = nullptr;
	m_resource.Reset();
}

Microsoft::WRL::ComPtr<ID3D12Resource> D3D12Resource::GetResourceComPtr()
{
	return m_resource;
}

void D3D12Resource::EmplaceSRV(const D3D12DescriptorHandle& InSRVHandle)
{
	m_srvHandle = InSRVHandle;
}

void D3D12Resource::EmplaceRTV(const D3D12DescriptorHandle& InRTVHandle)
{
	m_rtvHandle = InRTVHandle;
}

void D3D12Resource::EmplaceUAV(const D3D12DescriptorHandle& InUAVHandle)
{
	m_uavHandle = InUAVHandle;
}

Ideal::D3D12DescriptorHandle D3D12Resource::GetSRV() const
{
	if (m_srvHandle.GetCpuHandle().ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		__debugbreak();
	}
	return m_srvHandle;
}

Ideal::D3D12DescriptorHandle D3D12Resource::GetRTV() const
{
	if (m_rtvHandle.GetCpuHandle().ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		__debugbreak();
	}
	return m_rtvHandle;
}

Ideal::D3D12DescriptorHandle D3D12Resource::GetUAV() const
{
	if (m_uavHandle.GetCpuHandle().ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		__debugbreak();
	}
	return m_uavHandle;
}

void D3D12Resource::EmplaceSRV2(const D3D12DescriptorHandle2& InSRVHandle)
{
	m_srvHandle2 = InSRVHandle;
}

void D3D12Resource::EmplaceRTV2(const D3D12DescriptorHandle2& InRTVHandle)
{
	m_rtvHandle2 = InRTVHandle;
}

void D3D12Resource::EmplaceUAV2(const D3D12DescriptorHandle2& InUAVHandle)
{
	m_uavHandle2 = InUAVHandle;
}

Ideal::D3D12DescriptorHandle2 D3D12Resource::GetSRV2() const
{
	return m_srvHandle2;
}

Ideal::D3D12DescriptorHandle2 D3D12Resource::GetRTV2() const
{
	return m_rtvHandle2;
}

Ideal::D3D12DescriptorHandle2 D3D12Resource::GetUAV2() const
{
	return m_uavHandle2;
}

void D3D12Resource::FreeHandle()
{
	m_srvHandle2.Free();
	m_rtvHandle2.Free();
	m_uavHandle2.Free();

	m_resource.Reset();
}

//------------------------UploadBuffer------------------------//

D3D12UploadBuffer::D3D12UploadBuffer()
	: m_bufferSize(0)
{

}

D3D12UploadBuffer::~D3D12UploadBuffer()
{
	if (m_resource.Get())
	{
		//m_resource->Unmap(0, nullptr);
	}
}

void Ideal::D3D12UploadBuffer::Create(ID3D12Device* Device, uint32 BufferSize)
{
	// 버퍼 사이즈
	m_bufferSize = BufferSize;

	// 업로드 용으로 GPU Upload heap에 버퍼를 만든다
	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf())
	), L"Failed to create upload buffer");
}

void* D3D12UploadBuffer::MapCpuWriteOnly()
{
	void* mappedData;
	CD3DX12_RANGE readRange(0, 0);
	Check(m_resource->Map(0, &readRange, &mappedData), L"Failed to mapped data");
	return mappedData;
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

//------------------------GPUBuffer------------------------//

D3D12GPUBuffer::D3D12GPUBuffer()
	: m_bufferSize(0),
	m_elementSize(0),
	m_elementCount(0)
{

}

D3D12GPUBuffer::~D3D12GPUBuffer()
{

}

void Ideal::D3D12GPUBuffer::CreateBuffer(ID3D12Device* Device, uint32 ElementSize, uint32 ElementCount, D3D12_RESOURCE_FLAGS Flags /*= D3D12_RESOURCE_FLAG_NONE*/)
{
	m_bufferSize = ElementSize * ElementCount;
	m_elementSize = ElementSize;
	m_elementCount = ElementCount;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize, Flags);
	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		//D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COMMON,
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

uint32 D3D12GPUBuffer::GetElementSize() const
{
	return m_elementSize;
}

void D3D12GPUBuffer::SetName(const LPCTSTR& name)
{
	m_name = name;
	m_resource->SetName(name);
}

//------------------------VertexBuffer------------------------//

D3D12VertexBuffer::D3D12VertexBuffer()
	: m_vertexBufferView()
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
	D3D12GPUBuffer::SetName(L"VertexBuffer");
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

void D3D12VertexBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount);
	D3D12GPUBuffer::SetName(L"VertexBuffer");
	// 데이터를 복사한다
	CmdList->CopyBufferRegion(m_resource.Get(), 0, UploadBuffer->GetResource(), 0, m_bufferSize);

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

void Ideal::D3D12VertexBuffer::CreateAndCopyResource(ComPtr<ID3D12Device> Device, uint32 ElementSize, uint32 ElementCount, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12Resource> Resource, D3D12_RESOURCE_STATES BeforeState)
{
	//m_resource = Resource->GetResourceComPtr();
	D3D12GPUBuffer::CreateBuffer(Device.Get(), ElementSize, ElementCount);
	D3D12GPUBuffer::SetName(L"VertexBuffer");

	CD3DX12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		Resource->GetResource(),
		BeforeState,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	
	CommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
	CommandList->CopyResource(m_resource.Get(), Resource->GetResource());

	CD3DX12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		Resource->GetResource(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		BeforeState
	);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	);

	CommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
	
	m_vertexBufferView.BufferLocation = m_resource->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = m_bufferSize;
	m_vertexBufferView.StrideInBytes = m_elementSize;
}

//------------------------IndexBuffer------------------------//

D3D12IndexBuffer::D3D12IndexBuffer()
	: m_indexBufferView()
{

}

D3D12IndexBuffer::~D3D12IndexBuffer()
{

}

void D3D12IndexBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount);
	D3D12GPUBuffer::SetName(L"IndexBuffer");

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

void D3D12IndexBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, std::shared_ptr<D3D12UploadBuffer> UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount);
	D3D12GPUBuffer::SetName(L"IndexBuffer");

	CmdList->CopyBufferRegion(m_resource.Get(), 0, UploadBuffer->GetResource(), 0, m_bufferSize);

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

//------------------------ConstantBuffer------------------------//

D3D12ConstantBuffer::D3D12ConstantBuffer()
	: m_bufferSize(0),
	m_perFrameBufferSize(0),
	m_mappedConstantBuffer(nullptr),
	m_isMapped(false)
{

}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
	if (m_isMapped)
	{
		m_resource->Unmap(0, nullptr);
	}
}

void Ideal::D3D12ConstantBuffer::Create(ID3D12Device* Device, uint32 BufferSize, uint32 FrameCount)
{
	// constant buffer 사이즈는 256 바이트 배수여야 한다

	m_perFrameBufferSize = Align(BufferSize);
	m_bufferSize = m_perFrameBufferSize * FrameCount;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
	
	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_resource.GetAddressOf())
	));
	m_resource->SetName(L"ConstantBuffer");
	 
	CD3DX12_RANGE readRange(0, 0);
	Check(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
	m_isMapped = true;
}

D3D12_GPU_VIRTUAL_ADDRESS D3D12ConstantBuffer::GetGPUVirtualAddress(uint32 FrameIndex)
{
	uint32 beginDataOffset = m_perFrameBufferSize * FrameIndex;
	return m_resource->GetGPUVirtualAddress() + beginDataOffset;
}

void* D3D12ConstantBuffer::GetMappedMemory(uint32 FrameIndex)
{
	// 현재 프레임에 할당된 메모리의 주소를 가져온다.
	uint32 beginDataOffset = m_perFrameBufferSize * FrameIndex;
	uint8* beginData = (uint8*)m_mappedConstantBuffer + beginDataOffset;

	return beginData;
}

uint32 D3D12ConstantBuffer::Align(uint32 location, uint32 align /*= D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT*/)
{
	return (location + (align - 1)) & ~(align - 1);
}

//------------------------UAV Buffer------------------------//

D3D12UAVBuffer::D3D12UAVBuffer()
{

}

D3D12UAVBuffer::~D3D12UAVBuffer()
{

}

void D3D12UAVBuffer::Create(ID3D12Device* Device, uint32 BufferSize, D3D12_RESOURCE_STATES InitialResourceState/*= D3D12_RESOURCE_STATE_COMMON*/, const wchar_t* Name /*= nullptr*/)
{
	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	Check(Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		InitialResourceState,
		nullptr,
		IID_PPV_ARGS(&m_resource))
		,
		L"Failed to create UAV Buffer!"
	);

	if (Name)
	{
		m_resource->SetName(Name);
	}
}

D3D12_GPU_VIRTUAL_ADDRESS D3D12UAVBuffer::GetGPUVirtualAddress()
{
	return m_resource->GetGPUVirtualAddress();
}

void D3D12UAVBuffer::SetUAV(std::shared_ptr<Ideal::D3D12UnorderedAccessView> UAV)
{
	m_uav = UAV;
}

std::shared_ptr<Ideal::D3D12UnorderedAccessView> D3D12UAVBuffer::GetUAV()
{
	return m_uav;
}

void D3D12StructuredBuffer::Free()
{
	m_srvHandle.Free();
	m_uavHandle.Free();
}

void D3D12StructuredBuffer::EmplaceSRV(Ideal::D3D12DescriptorHandle SRVHandle)
{
	m_srvHandle = SRVHandle;
}

void D3D12StructuredBuffer::EmplaceUAV(Ideal::D3D12DescriptorHandle UAVHandle)
{
	m_uavHandle = UAVHandle;
}

void D3D12StructuredBuffer::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, uint32 ElementSize, uint32 ElementCount, const D3D12UploadBuffer& UploadBuffer)
{
	D3D12GPUBuffer::CreateBuffer(Device, ElementSize, ElementCount, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12GPUBuffer::SetName(L"StructuredBuffer");

	// 데이터를 복사한다
	CmdList->CopyBufferRegion(m_resource.Get(), 0, UploadBuffer.GetResource(), 0, m_bufferSize);

	CD3DX12_RESOURCE_BARRIER resourceBarrier
		= CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

	CmdList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12StructuredBuffer::TransitionToSRV(ID3D12GraphicsCommandList* CmdList)
{
	CD3DX12_RESOURCE_BARRIER resourceBarrier
		= CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

	CmdList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12StructuredBuffer::TransitionToUAV(ID3D12GraphicsCommandList* CmdList)
{
	CD3DX12_RESOURCE_BARRIER resourceBarrier
		= CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);

	CmdList->ResourceBarrier(1, &resourceBarrier);
}

Ideal::D3D12DescriptorHandle D3D12StructuredBuffer::GetSRV()
{
	return m_srvHandle;
}

Ideal::D3D12DescriptorHandle D3D12StructuredBuffer::GetUAV()
{
	return m_uavHandle;
}
