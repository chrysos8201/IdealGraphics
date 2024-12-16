#pragma once
#include "Core/Core.h"

#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

#include "Misc/Assimp/AssimpConvertType.h"

namespace AssimpConvert
{
	struct Mesh;
	struct Material;
	struct Bone;
}

class AssimpConverter
{
	struct AABB {
		aiVector3D minBounds;
		aiVector3D maxBounds;
	};

public:
	AssimpConverter();
	~AssimpConverter();

public:
	static std::wstring ConvertStringToWString(const std::string& str);
	static std::string ConvertWStringToString(const std::wstring& wstr);
	static void Replace(std::string& OutStr, std::string Comp, std::string Rep);
	static void Replace(std::wstring& OutStr, std::wstring Comp, std::wstring Rep);

public:
	void SetAssetPath(const std::wstring& AssetPath) { m_assetPath = AssetPath; }
	void SetModelPath(const std::wstring& ModelPath) { m_modelPath = ModelPath; }
	void SetTexturePath(const std::wstring& TexturePath) { m_texturePath = TexturePath; }

public:
	void ReadAssetFile(const std::wstring& path, bool isSkinnedData, bool onlyPosition);
	void ExportModelData(std::wstring savePath, bool IsSkinnedData = false, bool IsConvertCenter = false);
	void ExportMaterialData(const std::wstring& savePath);
	void ExportAnimationData(std::wstring savePath, uint32 index = 0);

	// 정점 위치만 내보내기 위한 함수
	void ExportVertexPositionData(const std::wstring& savePath);

	// 파티클 매쉬 
	void ExportParticleData(std::wstring savePath, bool SetScale = false, Vector3 Scale = Vector3(1.f));



private:
	std::string WriteTexture(std::string SaveFolder, std::string File);
	void WriteMaterialData(std::wstring FilePath);
	void WriteModelFile(const std::wstring& filePath);
	void WriteSkinnedModelFile(const std::wstring& filePath);
	void ReadModelData(aiNode* node, int32 index, int32 parent, bool isNegative = false);
	void ReadSkinnedModelData(aiNode* node, int32 index, int32 parent, bool readMeshData = true, bool converCenter = false);
	void ReadMaterialData();
	void ReadSkinData();
	void ReadMeshData(aiNode* node, int32 bone, Vector3 scale, bool convertCenter = false);
	void ReadSkinnedMeshData(aiNode* node, int32 bone, int32 parentBone = -1);

	void ReadParticleModelData(aiNode* node, int32 index, int32 parent, bool SetScale, Vector3 Scale = Vector3(1.f));
	void WriteParticleModelData(const std::wstring& filePath);

	std::shared_ptr<AssimpConvert::Animation> ReadAnimationData(aiAnimation* srcAnimation);
	std::shared_ptr<AssimpConvert::AnimationNode> ParseAnimationNode(std::shared_ptr<AssimpConvert::Animation> animation, aiNodeAnim* srcNode);
	void ReadKeyFrameData(std::shared_ptr<AssimpConvert::Animation> animation, aiNode* node, std::map<std::string, std::shared_ptr<AssimpConvert::AnimationNode>>& cache);
	void WriteAnimationData(std::shared_ptr<AssimpConvert::Animation> animation, std::wstring finalPath);
	uint32 GetBoneIndex(const std::string& name);

	void WriteVertexPositionFile(const std::wstring& filePath);

public:
	AABB CalculateAABB(const aiMesh* mesh);

private:
	
	std::shared_ptr<Assimp::Importer> m_importer;
	const aiScene* m_scene;

	std::wstring m_assetPath = L"Resources/Assets/";
	std::wstring m_modelPath = L"Resources/Models/";
	std::wstring m_texturePath = L"Resources/Textures/";

private:
	bool m_isSkinnedData = false;

	std::vector<std::shared_ptr<AssimpConvert::Material>> m_materials;
	std::vector<std::shared_ptr<AssimpConvert::Mesh>> m_meshes;
	std::vector<std::shared_ptr<AssimpConvert::ParticleMesh>> m_particleMeshes;
	std::vector<std::shared_ptr<AssimpConvert::SkinnedMesh>> m_skinnedMeshes;
	std::vector<std::shared_ptr<AssimpConvert::Bone>> m_bones;
};