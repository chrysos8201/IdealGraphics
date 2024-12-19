#include "IdealStaticMesh.h"

#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/Resource/IdealBone.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"

#include "GraphicsEngine/D3d12/D3D12ConstantBufferPool.h"

Ideal::IdealStaticMesh::IdealStaticMesh()
{

}

Ideal::IdealStaticMesh::~IdealStaticMesh()
{

}

void Ideal::IdealStaticMesh::Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer, const Matrix& WorldTM)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	ComPtr<ID3D12Device> device = d3d12Renderer->GetDevice();

	//int i = 0;
	//if (m_meshes.size() > 1) i = 0;
	//auto& mesh = m_meshes[i];
	for (auto& mesh : m_meshes)
	{
		std::shared_ptr<Ideal::D3D12DescriptorHeap> descriptorHeap = d3d12Renderer->GetMainDescriptorHeap();
		auto cb = d3d12Renderer->ConstantBufferAllocate(sizeof(CB_Transform));
		if (!cb)
		{
			__debugbreak();
		}

		Matrix finalTM = mesh->GetLocalTM() * WorldTM;

		CB_Transform* cbTransform = (CB_Transform*)cb->SystemMemoryAddress;
		cbTransform->World = finalTM.Transpose();
		cbTransform->WorldInvTranspose = finalTM.Transpose().Invert();

		// b0용 Descriptor Table 할당
		auto handle = descriptorHeap->Allocate(1);	// 공용 root parameter인 b0 : transform 만 필요하니 Descriptor 공간을 하나만 받는다.
		uint32 incrementSize = d3d12Renderer->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Copy To Main Descriptor
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(handle.GetCpuHandle(), STATIC_MESH_DESCRIPTOR_INDEX_CBV_TRANSFORM, incrementSize);
		device->CopyDescriptorsSimple(1, cbvDest, cb->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		// 공용으로 쓰는 Root Parameter인 Transform은 Root Paramter Index가 0번임.
		commandList->SetGraphicsRootDescriptorTable(STATIC_MESH_DESCRIPTOR_TABLE_INDEX_OBJ, handle.GetGpuHandle());

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
