#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"

Ideal::D3D12Texture::D3D12Texture()
{

}

Ideal::D3D12Texture::~D3D12Texture()
{

}

void Ideal::D3D12Texture::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, const std::wstring& Path, D3D12_CPU_DESCRIPTOR_HANDLE SRVHeapHandle)
{
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
	uint64 bufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);

	//----------------------Upload Buffer--------------------------//

	Ideal::D3D12UploadBuffer uploadBuffer;
	uploadBuffer.Create(Device, bufferSize);

	//----------------------Update Subresources--------------------------//

	UpdateSubresources(
		CommandList,
		m_resource.Get(),
		uploadBuffer.GetResource(),
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


}
