#include "Misc/AssimpLoader.h"
#include "assimp/importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/types.h"
#include "assimp/mesh.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "RenderTest/VertexInfo.h"
#include "RenderTest/MeshTest.h"
#include "RenderTest/ModelTest.h"

#include <filesystem>

bool AssimpLoader::Load(ImportSettings settings)
{
	auto& meshes = settings.meshes;
	bool inverseU = settings.inverseU;
	bool inverseV = settings.inverseV;

	Assimp::Importer importer;
	int32 flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;

	auto scene = importer.ReadFile(settings.filename, flag);

	if (scene == nullptr)
	{
		printf(importer.GetErrorString());
		return false;
	}

	meshes.clear();
	meshes.resize(scene->mNumMeshes);
	for (uint32 i = 0; i < meshes.size(); i++)
	{
		const auto mesh = scene->mMeshes[i];
		LoadMesh(meshes[i], mesh, inverseU, inverseV);
		const auto material = scene->mMaterials[i];
		LoadTexture(settings.filename, meshes[i], material);
	}

	scene = nullptr;

	return false;
}

bool AssimpLoader::Load(ImportSettings2 settings)
{
	auto& meshes = settings.meshes;
	bool inverseU = settings.inverseU;
	bool inverseV = settings.inverseV;

	Assimp::Importer importer;
	int32 flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;

	auto scene = importer.ReadFile(settings.filename, flag);

	if (scene == nullptr)
	{
		printf(importer.GetErrorString());
		return false;
	}

	meshes.clear();
	meshes.resize(scene->mNumMeshes);
	for (uint32 i = 0; i < meshes.size(); i++)
	{
		const auto mesh = scene->mMeshes[i];
		LoadMesh(meshes[i], mesh, inverseU, inverseV);
		const auto material = scene->mMaterials[i];
		LoadTexture(settings.filename, meshes[i], material);
	}

	scene = nullptr;

	return true;
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV)
{
	aiVector3D zero3D(0.f, 0.f, 0.f);
	aiColor4D zeroColor(0.f, 0.f, 0.f, 0.f);

	dst.vertices.resize(src->mNumVertices);

	for (uint32 i = 0; i < src->mNumVertices; i++)
	{
		auto position = &(src->mVertices[i]);
		auto normal = &(src->mNormals[i]);
		auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
		auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;
		auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;

		if (inverseU)
		{
			uv->x = 1 - uv->x;
		}
		if (inverseV)
		{
			uv->y = 1 - uv->y;
		}

		Vertex vertex = {};
		vertex.Position = Vector3(position->x, position->y, position->z);
		vertex.Normal = Vector3(normal->x, normal->y, normal->z);
		vertex.UV = Vector2(uv->x, uv->y);
		vertex.Tangent = Vector3(tangent->x, tangent->y, tangent->z);
		vertex.Color = Vector4(color->r, color->g, color->b, color->a);

		dst.vertices[i] = vertex;
	}

	dst.indices.resize(src->mNumFaces * 3);
	
	for (uint32 i = 0; i < src->mNumFaces; i++)
	{
		const auto& face = src->mFaces[i];

		dst.indices[i * 3 + 0] = face.mIndices[0];
		dst.indices[i * 3 + 1] = face.mIndices[1];
		dst.indices[i * 3 + 2] = face.mIndices[2];
	}
}

void AssimpLoader::LoadMesh(MeshTest& dst, const aiMesh* src, bool inverseU, bool inverseV)
{
	aiVector3D zero3D(0.f, 0.f, 0.f);
	aiColor4D zeroColor(0.f, 0.f, 0.f, 0.f);

	dst.m_vertices.resize(src->mNumVertices);

	for (uint32 i = 0; i < src->mNumVertices; i++)
	{
		auto position = &(src->mVertices[i]);
		auto normal = &(src->mNormals[i]);
		auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
		auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;
		auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;

		if (inverseU)
		{
			uv->x = 1 - uv->x;
		}
		if (inverseV)
		{
			uv->y = 1 - uv->y;
		}

		Vertex vertex = {};
		vertex.Position = Vector3(position->x, position->y, position->z);
		vertex.Normal = Vector3(normal->x, normal->y, normal->z);
		vertex.UV = Vector2(uv->x, uv->y);
		vertex.Tangent = Vector3(tangent->x, tangent->y, tangent->z);
		vertex.Color = Vector4(color->r, color->g, color->b, color->a);

		dst.m_vertices[i] = vertex;
	}

	dst.m_indices.resize(src->mNumFaces * 3);

	for (uint32 i = 0; i < src->mNumFaces; i++)
	{
		const auto& face = src->mFaces[i];

		dst.m_indices[i * 3 + 0] = face.mIndices[0];
		dst.m_indices[i * 3 + 1] = face.mIndices[1];
		dst.m_indices[i * 3 + 2] = face.mIndices[2];
	}
}

void AssimpLoader::LoadTexture(const std::string& filename, Mesh& dst, const aiMaterial* src)
{
	aiString path;
	if (src->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		//auto dir = GetDi(filename);
	}
	else
	{
		dst.diffuseMap.clear();
	}
}

void AssimpLoader::LoadTexture(const std::string& filename, MeshTest& dst, const aiMaterial* src)
{
	aiString path;
	if (src->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		//auto dir = GetDi(filename);
	}
	else
	{
		dst.m_diffuseMap.clear();
	}
}