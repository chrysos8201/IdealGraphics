#include "Core/Core.h"
#include "IdealSkinnedMesh.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/Resource/IdealBone.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"
Ideal::IdealSkinnedMesh::IdealSkinnedMesh()
{

}

Ideal::IdealSkinnedMesh::~IdealSkinnedMesh()
{

}

void Ideal::IdealSkinnedMesh::Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	for (auto& mesh : m_meshes)
	{
		// Mesh
		const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = mesh->GetVertexBufferView();
		const D3D12_INDEX_BUFFER_VIEW& indexBufferView = mesh->GetIndexBufferView();

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		// Material
		std::weak_ptr<Ideal::IdealMaterial> material = mesh->GetMaterial();
		if (!material.expired())
		{
			material.lock()->BindToShader(d3d12Renderer);
		}
		// Final Draw
		commandList->DrawIndexedInstanced(mesh->GetElementCount(), 1, 0, 0, 0);
	}
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
