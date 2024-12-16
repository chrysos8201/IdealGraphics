#include "GraphicsEngine/Resource/Particle/ParticleMaterial.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/Resource/ShaderManager.h"

void Ideal::ParticleMaterial::SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode BlendingMode)
{
	m_blendingMode = BlendingMode;
}

void Ideal::ParticleMaterial::SetShader(std::shared_ptr<Ideal::IShader> Shader)
{
	m_shader = std::static_pointer_cast<Ideal::D3D12Shader>(Shader);
}

void Ideal::ParticleMaterial::SetShaderByPath(const std::wstring& FilePath)
{
	std::shared_ptr<Ideal::ShaderManager> shaderManager = std::make_shared<Ideal::ShaderManager>();
	shaderManager->Init();
	shaderManager->LoadShaderFile(FilePath, m_shader);
}

void Ideal::ParticleMaterial::SetTexture0(std::shared_ptr<Ideal::ITexture> Texture)
{
	m_texture0 = std::static_pointer_cast<Ideal::D3D12Texture>(Texture);
}

void Ideal::ParticleMaterial::SetTexture1(std::shared_ptr<Ideal::ITexture> Texture)
{
	m_texture1 = std::static_pointer_cast<Ideal::D3D12Texture>(Texture);
}

void Ideal::ParticleMaterial::SetTexture2(std::shared_ptr<Ideal::ITexture> Texture)
{
	m_texture2 = std::static_pointer_cast<Ideal::D3D12Texture>(Texture);
}

void Ideal::ParticleMaterial::SetBackFaceCulling(bool Culling)
{
	m_isBackFaceCulling = Culling;
}

void Ideal::ParticleMaterial::SetWriteDepthBuffer(bool Write)
{
	m_isWriteDepthBuffer = Write;
}

void Ideal::ParticleMaterial::SetTransparency(bool Transpaerancy)
{
	m_isTransperancy = Transpaerancy;
}

Ideal::ParticleMaterialMenu::EBlendingMode Ideal::ParticleMaterial::GetBlendingMode()
{
	return m_blendingMode;
}

std::shared_ptr<Ideal::D3D12Shader> Ideal::ParticleMaterial::GetShader()
{
	return m_shader;
}

std::weak_ptr<Ideal::D3D12Texture> Ideal::ParticleMaterial::GetTexture0()
{
	return m_texture0;
}

std::weak_ptr<Ideal::D3D12Texture> Ideal::ParticleMaterial::GetTexture1()
{
	return m_texture1;
}

std::weak_ptr<Ideal::D3D12Texture> Ideal::ParticleMaterial::GetTexture2()
{
	return m_texture2;
}

bool Ideal::ParticleMaterial::GetBackFaceCulling()
{
	return m_isBackFaceCulling;
}

bool Ideal::ParticleMaterial::GetWriteDepthBuffer()
{
	return m_isWriteDepthBuffer;
}

bool Ideal::ParticleMaterial::GetTransparency()
{
	return m_isTransperancy;
}
