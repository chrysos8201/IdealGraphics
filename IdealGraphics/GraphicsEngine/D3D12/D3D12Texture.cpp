#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"
#include "GraphicsEngine/IdealRenderer.h"

Ideal::D3D12Texture::D3D12Texture()
{

}

Ideal::D3D12Texture::~D3D12Texture()
{

}

void Ideal::D3D12Texture::Create(std::shared_ptr<IdealRenderer> Renderer, const std::wstring& Path)
{
	// TODO : SET NAME

	//----------------------Load WIC Texture From File--------------------------//

	std::unique_ptr<uint8_t[]> decodeData;
	D3D12_SUBRESOURCE_DATA subResource;
	Check(DirectX::LoadWICTextureFromFile(
		Renderer->GetDevice().Get(),
		Path.c_str(),
		m_resource.ReleaseAndGetAddressOf(),
		decodeData,
		subResource
	));
	// GetSize
	//uint64 bufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);

	//----------------------Upload Buffer--------------------------//

	uint64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	ComPtr<ID3D12Resource> uploadBuffer;
	Check(Renderer->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	//Ideal::D3D12UploadBuffer uploadBuffer;
	//uploadBuffer.Create(Device, bufferSize);

	//----------------------Update Subresources--------------------------//

	UpdateSubresources(
		Renderer->GetCommandList().Get(),
		m_resource.Get(),
		uploadBuffer.Get(),
		0, 0, 1, &subResource
	);

	//----------------------Resource Barrier--------------------------//
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	Renderer->GetCommandList()->ResourceBarrier(1, &barrier);

	//----------------------Create Shader Resource View--------------------------//

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	
	// 2024.04.21 : Renderer에서 미리 크게 만들어둔 SRV Descriptor Heap의 주소만 가져온다.
	m_srvHandle = Renderer->AllocateSRV();
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHandle.GetCpuHandle();

	Renderer->GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, cpuHandle);

	Renderer->ExecuteCommandList();
}

void Ideal::D3D12Texture::Create(ComPtr<ID3D12Resource> Resource, Ideal::D3D12DescriptorHandle Handle)
{
	m_resource = Resource;
	m_srvHandle = Handle;
}
