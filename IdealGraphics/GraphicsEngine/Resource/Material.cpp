#include "GraphicsEngine/Resource/Material.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"

Ideal::Material::Material()
{

}

Ideal::Material::~Material()
{

}

void Ideal::Material::Create(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, D3D12_CPU_DESCRIPTOR_HANDLE SRVHeapHandle)
{
	m_diffuseTexture = std::make_shared<Ideal::D3D12Texture>();
	m_diffuseTexture->Create(Device, CommandList, SRVHeapHandle, m_diffuseTextureFile);

}
