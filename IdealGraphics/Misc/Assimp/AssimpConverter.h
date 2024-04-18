#pragma once
#include "Core/Core.h"

#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

#include "Misc/Assimp/AssimpConvertType.h"

namespace AssimpConvert
{
	class Mesh;
	class Material;
	class Bone;
}

class AssimpConverter
{
public:
	AssimpConverter();
	~AssimpConverter();

public:
	static std::wstring ConvertStringToWString(const std::string& str);
	static std::string ConvertWStringToString(const std::wstring& wstr);
	static void Replace(std::string& OutStr, std::string Comp, std::string Rep);
	static void Replace(std::wstring& OutStr, std::wstring Comp, std::wstring Rep);

public:
	void ReadAssetFile(const std::wstring& path);
	void ExportModelData(std::wstring savePath);
	void ExportMaterialData(const std::wstring& savePath);

private:
	std::string WriteTexture(std::string SaveFolder, std::string File);
	void WriteMaterialData(std::wstring FilePath);
	void WriteModelFile(const std::wstring& filePath);
	void ReadModelData(aiNode* node, int32 index, int32 parent);
	void ReadMaterialData();
	void ReadMeshData(aiNode* node, int32 bone);

private:
	std::shared_ptr<Assimp::Importer> m_importer;
	const aiScene* m_scene;

	std::wstring m_assetPath = L"Resources/Assets/";
	std::wstring m_modelPath = L"Resources/Models/";
	std::wstring m_texturePath = L"Resources/Textures/";

private:
	std::vector<std::shared_ptr<AssimpConvert::Material>> m_materials;
	std::vector<std::shared_ptr<AssimpConvert::Mesh>> m_meshes;
	std::vector<std::shared_ptr<AssimpConvert::Bone>> m_bones;
};