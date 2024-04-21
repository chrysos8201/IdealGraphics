#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

class IdealRenderer;

namespace Ideal
{
	class D3D12Texture;

	class Material : public ResourceBase
	{
		friend class AssimpConverter;
		friend class Model;

	public:
		Material();
		virtual ~Material();

		void SetAmbient(Color c) { m_ambient = c; }
		void SetDiffuse(Color c) { m_diffuse = c; }
		void SetSpecular(Color c) { m_specular = c; }
		void SetEmissive(Color c) { m_emissive = c; }

		void Create(std::shared_ptr<IdealRenderer> Renderer);
		// Ω¶¿Ã¥ı∂˚ πŸ¿ŒµÂ∏¶ «—¥Ÿ.
		void BindToShader(std::shared_ptr<IdealRenderer> Renderer);

	private:
		Color m_ambient;
		Color m_specular;
		Color m_diffuse;
		Color m_emissive;

		std::wstring m_diffuseTextureFile;
		std::wstring m_specularTextureFile;
		std::wstring m_emissiveTextureFile;
		std::wstring m_normalTextureFile;

		std::shared_ptr<Ideal::D3D12Texture> m_diffuseTexture;
		std::shared_ptr<Ideal::D3D12Texture> m_specularTexture;
		std::shared_ptr<Ideal::D3D12Texture> m_emissiveTexture;
		std::shared_ptr<Ideal::D3D12Texture> m_normalTexture;
	};
}

