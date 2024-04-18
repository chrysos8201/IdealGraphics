#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

namespace Ideal
{

	class Material : public ResourceBase
	{
		friend class AssimpConverter;

	public:
		Material();
		virtual ~Material();

		void SetAmbient(Color c) { m_ambient = c; }
		void SetDiffuse(Color c) { m_diffuse = c; }
		void SetSpecular(Color c) { m_specular = c; }
		void SetEmissive(Color c) { m_emissive = c; }
	private:
		Color m_ambient;
		Color m_specular;
		Color m_diffuse;
		Color m_emissive;

		std::string m_diffuseTextureFile;
		std::string m_specularTextureFile;
		std::string m_emissiveTextureFile;
		std::string m_normalTextureFile;
	};
}

