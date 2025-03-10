#include "Core/Core.h"
#include "IdealSkinnedMesh.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/Resource/IdealBone.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
Ideal::IdealSkinnedMesh::IdealSkinnedMesh()
{

}

Ideal::IdealSkinnedMesh::~IdealSkinnedMesh()
{

}

void Ideal::IdealSkinnedMesh::Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer)
{
	
}

void Ideal::IdealSkinnedMesh::AddMesh(std::shared_ptr<Ideal::IdealMesh<SkinnedVertex>> Mesh)
{
	m_meshes.push_back(Mesh);
}

void Ideal::IdealSkinnedMesh::AddMaterial(std::shared_ptr<Ideal::IdealMaterial> Material)
{
	for (auto& mesh : m_meshes)
	{
		// 이미 material이 바인딩 되어 있을 경우
		if (!mesh->GetMaterial().expired())
		{
			continue;
		}

		if (Material->GetName() == mesh->GetMaterialName())
		{
			mesh->SetMaterial(Material);
		}
	}
}

void Ideal::IdealSkinnedMesh::FinalCreate(std::shared_ptr<Ideal::ResourceManager> ResourceManager)
{
	for (auto& mesh : m_meshes)
	{
		mesh->Create(ResourceManager);
	}
}
