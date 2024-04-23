#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"
#include "GraphicsEngine/IdealRenderer.h"

Ideal::D3D12Texture::D3D12Texture()
{

}

Ideal::D3D12Texture::~D3D12Texture()
{

}

void Ideal::D3D12Texture::Create(ComPtr<ID3D12Resource> Resource, Ideal::D3D12DescriptorHandle Handle)
{
	m_resource = Resource;
	m_srvHandle = Handle;
}
