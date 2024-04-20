#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"
#include "GraphicsEngine/IdealRenderer.h"

Ideal::D3D12Texture::D3D12Texture()
{

}

Ideal::D3D12Texture::~D3D12Texture()
{

}

void Ideal::D3D12Texture::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, D3D12_CPU_DESCRIPTOR_HANDLE SRVHeapHandle, const std::wstring& Path, std::shared_ptr<IdealRenderer> Renderer)
{
	//std::unique_ptr<uint8_t[]> decodedData;
	//D3D12_SUBRESOURCE_DATA subResource;
	//Check(DirectX::LoadWICTextureFromFile(
	//	Device,
	//	L"Resources/Test/test2.png",
	//	m_resource.ReleaseAndGetAddressOf(),
	//	decodedData,
	//	subResource
	//));

	////----------------Create Upload Buffer----------------//

	//uint64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
	//CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
	//CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	//ComPtr<ID3D12Resource> uploadBuffer;
	//Check(Device->CreateCommittedResource(
	//	&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&resourceDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	//));

	////----------------Update Subresources----------------//

	//UpdateSubresources(
	//	CommandList, m_resource.Get(),
	//	uploadBuffer.Get(),
	//	0, 0, 1, &subResource
	//);
	//m_resource->SetName(L"Test");

	////----------------Resource Barrier----------------//

	//CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	//	m_resource.Get(),
	//	D3D12_RESOURCE_STATE_COPY_DEST,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	//);
	//CommandList->ResourceBarrier(1, &barrier);

	////----------------Create Shader Resource View----------------//

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.Format = m_resource->GetDesc().Format;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MipLevels = 1;
	//Device->CreateShaderResourceView(m_resource.Get(), &srvDesc, SRVHeapHandle);


	//Renderer->ExecuteCommandList();

	//return;


	// TODO : SET NAME

	//----------------------Load WIC Texture From File--------------------------//

	std::unique_ptr<uint8_t[]> decodeData;
	D3D12_SUBRESOURCE_DATA subResource;
	Check(DirectX::LoadWICTextureFromFile(
		Device,
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
	Check(Device->CreateCommittedResource(
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
		CommandList,
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
	CommandList->ResourceBarrier(1, &barrier);

	//----------------------Create Shader Resource View--------------------------//

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	
	Device->CreateShaderResourceView(m_resource.Get(), &srvDesc, SRVHeapHandle);

	Renderer->ExecuteCommandList();
}
