#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"

namespace Ideal
{

	class Material
	{
		friend class AssimpConverter;

	public:
		Material();
		virtual ~Material();

	private:
		std::string m_name;
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

