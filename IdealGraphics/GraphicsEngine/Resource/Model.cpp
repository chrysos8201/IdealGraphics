#include "GraphicsEngine/Resource/Model.h"
#include "Misc/Utils/tinyxml2.h"
#include <filesystem>
#include "Misc/Utils/StringUtils.h"

#include "GraphicsEngine/Resource/Bone.h"
#include "GraphicsEngine/Resource/Mesh.h"
#include "GraphicsEngine/Resource/Material.h"
#include "Misc/Utils/FileUtils.h"

#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/IdealRenderer.h"
Ideal::Model::Model()
{

}

Ideal::Model::~Model()
{

}

void Ideal::Model::ReadMaterial(const std::wstring& filename)
{
	std::wstring texturePath = L"Resources/Textures/";
	std::wstring fullPath = texturePath + filename + L".xml";

	std::filesystem::path parentPath = std::filesystem::path(fullPath).parent_path();

	tinyxml2::XMLDocument* document = new tinyxml2::XMLDocument();
	tinyxml2::XMLError error = document->LoadFile(StringUtils::ConvertWStringToString(fullPath).c_str());
	assert(error == tinyxml2::XML_SUCCESS);

	tinyxml2::XMLElement* root = document->FirstChildElement();
	tinyxml2::XMLElement* materialNode = root->FirstChildElement();

	while (materialNode)
	{
		std::shared_ptr<Ideal::Material> material = std::make_shared<Ideal::Material>();

		tinyxml2::XMLElement* node = nullptr;

		node = materialNode->FirstChildElement();
		material->SetName((node->GetText()));

		// DiffuseTexture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			std::wstring textureStr = StringUtils::ConvertStringToWString(node->GetText());
			if (textureStr.length() > 0)
			{
				// TODO : Texture�����
				//auto texture
			}
		}

		// Specular Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			std::wstring textureStr = StringUtils::ConvertStringToWString(node->GetText());
			if (textureStr.length() > 0)
			{
				// TODO : Texture�����
				//auto texture
			}
		}

		// Normal Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			std::wstring textureStr = StringUtils::ConvertStringToWString(node->GetText());
			if (textureStr.length() > 0)
			{
				// TODO : Texture�����
				//auto texture
			}
		}

		// Ambient
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->SetAmbient(color);
		}

		// Diffuse
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->SetDiffuse(color);
		}

		// Specular
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->SetSpecular(color);
		}

		// Emissive
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->SetEmissive(color);
		}

		m_materials.push_back(material);

		materialNode = materialNode->NextSiblingElement();
	}
}

void Ideal::Model::ReadModel(const std::wstring& filename)
{
	std::wstring fullPath = L"Resources/Models/";
	fullPath += filename + L".mesh";

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(fullPath, FileMode::Read);

	// bone
	{
		const uint32 count = file->Read<uint32>();
		
		for (uint32 i = 0; i < count; ++i)
		{
			std::shared_ptr<Ideal::Bone> bone = std::make_shared<Ideal::Bone>();
			bone->index = file->Read<int32>();
			//bone->name = StringUtils::ConvertStringToWString(file->Read<std::string>());
			bone->name = file->Read<std::string>();
			bone->parent = file->Read<int32>();
			bone->transform = file->Read<Matrix>();
			m_bones.push_back(bone);
		}
	}

	// Mesh
	{
		const uint32 count = file->Read<uint32>();

		for (uint32 i = 0; i < count; ++i)
		{
			std::shared_ptr <Ideal::Mesh> mesh = std::make_shared<Ideal::Mesh>();

			mesh->SetName(file->Read<std::string>());
			mesh->m_boneIndex = file->Read<int32>();

			// Material
			mesh->m_materialName = file->Read<std::string>();

			// vertex data
			{
				const uint32 count = file->Read<uint32>();
				std::vector<BasicVertex> vertices;
				vertices.resize(count);

				void* data = vertices.data();
				file->Read(&data, sizeof(BasicVertex) * count);
				mesh->AddVertices(vertices);
			}

			// index Data
			{
				const uint32 count = file->Read<uint32>();
				std::vector<uint32> indices;
				indices.resize(count);

				void* data = indices.data();
				file->Read(&data, sizeof(uint32) * count);
				mesh->AddIndices(indices);
			}

			// TODO : mesh Create
			//mesh->Create()
			m_meshes.push_back(mesh);
		}
	}
	//BindCacheInfo();
}

void Ideal::Model::Create(std::shared_ptr<IdealRenderer> Renderer)
{
	for (auto m : m_meshes)
	{
		m->Create(Renderer);
	}
}

void Ideal::Model::Tick()
{
	for (auto m : m_meshes)
	{
		m->Tick();
	}
}

void Ideal::Model::Render(ID3D12GraphicsCommandList* CommandList)
{
	for (auto& m : m_meshes)
	{
		m->Render(CommandList);
	}
}

void Ideal::Model::BindCacheInfo()
{
	for (const auto& mesh : m_meshes)
	{
		if (mesh->m_material != nullptr)
		{
			continue;
		}

		mesh->m_material = GetMaterialByName(mesh->m_materialName);
	}

	//for (const auto& mesh : m_meshes)
	//{
	//	if(mesh->m_bone != nullptr)
	//}
}

std::shared_ptr<Ideal::Material> Ideal::Model::GetMaterialByName(const std::string& name)
{
	for (auto& m : m_materials)
	{
		if (m->GetName() == name)
		{
			return m;
		}
	}
	return nullptr;
}

std::shared_ptr<Ideal::Mesh> Ideal::Model::GetMeshByName(const std::string& name)
{
	for (auto& m : m_meshes)
	{
		if (m->GetName() == name)
		{
			return m;
		}
	}
	return nullptr;
}

std::shared_ptr<Ideal::Bone> Ideal::Model::GetBoneByName(const std::string& name)
{
	for (auto& m : m_bones)
	{
		if (m->GetName() == name)
		{
			return m;
		}
	}
	return nullptr;
}