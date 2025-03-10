#include "IdealStaticMesh.h"

#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/Resource/IdealBone.h"

#include "GraphicsEngine/D3d12/D3D12ConstantBufferPool.h"

Ideal::IdealStaticMesh::IdealStaticMesh()
{

}

Ideal::IdealStaticMesh::~IdealStaticMesh()
{

}

void Ideal::IdealStaticMesh::Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer, const Matrix& WorldTM)
{
	
}

void Ideal::IdealStaticMesh::DebugDraw(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList)
{
	for (auto& mesh : m_meshes)
	{

		// Mesh
		const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = mesh->GetVertexBufferView();
		const D3D12_INDEX_BUFFER_VIEW& indexBufferView = mesh->GetIndexBufferView();

		CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		CommandList->IASetIndexBuffer(&indexBufferView);

		CommandList->DrawIndexedInstanced(mesh->GetElementCount(), 1, 0, 0, 0);
	}
}

void Ideal::IdealStaticMesh::AddMesh(std::shared_ptr<Ideal::IdealMesh<BasicVertex>> Mesh)
{
	m_meshes.push_back(Mesh);
}

void Ideal::IdealStaticMesh::AddMaterial(std::shared_ptr<Ideal::IdealMaterial> Material)
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
			continue;
		}
	}
}

void Ideal::IdealStaticMesh::FinalCreate(std::shared_ptr<Ideal::ResourceManager> ResourceManager)
{
	for (auto& mesh : m_meshes)
	{
		mesh->Create(ResourceManager);
	}
}

DirectX::SimpleMath::Matrix Ideal::IdealStaticMesh::GetLocalTM()
{
	return m_bones[1]->GetTransform();
}
