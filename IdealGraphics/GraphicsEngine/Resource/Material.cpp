#include "GraphicsEngine/Resource/Material.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/IdealRenderer.h"
#include "GraphicsEngine/D3D12/D3D12ResourceManager.h"

Ideal::Material::Material()
{

}

Ideal::Material::~Material()
{

}

void Ideal::Material::Create(std::shared_ptr<IdealRenderer> Renderer)
{
	if (m_diffuseTextureFile.length() > 0)
	{
		m_diffuseTexture = std::make_shared<Ideal::D3D12Texture>();
		//m_diffuseTexture->Create(Renderer, m_diffuseTextureFile);
		Renderer->GetResourceManager()->CreateTexture(m_diffuseTexture, m_diffuseTextureFile);
	}

	if (m_specularTextureFile.length() > 0)
	{
		//m_diffuseTexture
	}
}

void Ideal::Material::BindToShader(std::shared_ptr<IdealRenderer> Renderer)
{
	if (m_diffuseTexture)
	{
		Ideal::D3D12DescriptorHandle diffuseHandle = m_diffuseTexture->GetDescriptorHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE diffuseGPUAddress = diffuseHandle.GetGpuHandle();

		// 2024.04.21 : diffuse texture은 Shader에서 t0라고 가정하고 사용한다.
		// 그리고 이미 Renderer의 SRV의 Heap을 SetDescriptorHeap을 해주었다고 가정한다.
		Renderer->GetCommandList()->SetGraphicsRootDescriptorTable(0, diffuseGPUAddress);
	}
}
