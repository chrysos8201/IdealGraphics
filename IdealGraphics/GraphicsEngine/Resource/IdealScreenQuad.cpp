#include "IdealScreenQuad.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/D3D12/D3D12PipelineStateObject.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"
#include <d3d12.h>

Ideal::IdealScreenQuad::IdealScreenQuad()
{

}

Ideal::IdealScreenQuad::~IdealScreenQuad()
{

}

void Ideal::IdealScreenQuad::Init(std::shared_ptr<IdealRenderer> Renderer)
{
	m_vertices.push_back({ Vector3(-1.0f, -1.f, 0.1f),	Vector2(0.f, 1.f) });
	m_vertices.push_back({ Vector3(-1.f, 1.0f, 0.1f),	Vector2(0.f, 0.f) });
	m_vertices.push_back({ Vector3(1.f, -1.f, 0.1f),	Vector2(1.f, 1.f) });
	m_vertices.push_back({ Vector3(1.f, 1.f, 0.1f),		Vector2(1.f, 0.f) });
	m_indices = { 0, 1, 2, 2, 1, 3 };

	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	m_quadMesh = std::make_shared<Ideal::IdealMesh<QuadVertex>>();
	m_quadMesh->AddVertices(m_vertices);
	m_quadMesh->AddIndices(m_indices);
	m_quadMesh->Create(d3d12Renderer->GetResourceManager());
	//CreateRootSignature(Renderer);
	//CreatePipelineState(Renderer);
}

void Ideal::IdealScreenQuad::Draw(std::shared_ptr<IdealRenderer> Renderer, std::vector<std::shared_ptr<Ideal::D3D12Texture>>& GBufferTextures, std::shared_ptr<Ideal::D3D12Texture> DepthTexture)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12Device> device = d3d12Renderer->GetDevice();
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	std::shared_ptr<Ideal::D3D12DescriptorHeap> descriptorHeap = d3d12Renderer->GetMainDescriptorHeap();

	// Mesh
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_quadMesh->GetVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = m_quadMesh->GetIndexBufferView();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// Textures
	// SRV Bind
	{
		auto handle = descriptorHeap->Allocate(GBUFFER_SRV_NUM);		// t0 : Normal, t1 : Position, t2 : Diffuse...
		uint32 incrementSize = d3d12Renderer->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
		commandList->SetGraphicsRootDescriptorTable(SCREEN_DESCRIPTOR_TABLE_INDEX, handle.GetGpuHandle());
		
		// Albedo
		{
			Ideal::D3D12DescriptorHandle albedoHandle = GBufferTextures[0]->GetSRV();
			D3D12_CPU_DESCRIPTOR_HANDLE albedoCPUAddress = albedoHandle.GetCpuHandle();
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(handle.GetCpuHandle(), SCREEN_DESCRIPTOR_INDEX_SRV_ALBEDO, incrementSize);
			device->CopyDescriptorsSimple(1, srvDest, albedoCPUAddress, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		// Normal
		{
			Ideal::D3D12DescriptorHandle normalHandle = GBufferTextures[1]->GetSRV();
			D3D12_CPU_DESCRIPTOR_HANDLE normalCPUAddress = normalHandle.GetCpuHandle();
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(handle.GetCpuHandle(), SCREEN_DESCRIPTOR_INDEX_SRV_NORMAL, incrementSize);
			device->CopyDescriptorsSimple(1, srvDest, normalCPUAddress, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		// PosH
		{
			Ideal::D3D12DescriptorHandle posHomogeneousHandle = GBufferTextures[2]->GetSRV();
			D3D12_CPU_DESCRIPTOR_HANDLE posHomogeneousCPUAddress = posHomogeneousHandle.GetCpuHandle();
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(handle.GetCpuHandle(), SCREEN_DESCRIPTOR_INDEX_SRV_POSH, incrementSize);
			device->CopyDescriptorsSimple(1, srvDest, posHomogeneousCPUAddress, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		// POSW
		{
			Ideal::D3D12DescriptorHandle tangentHandle = GBufferTextures[3]->GetSRV();
			D3D12_CPU_DESCRIPTOR_HANDLE tangentCPUAddress = tangentHandle.GetCpuHandle();
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(handle.GetCpuHandle(), SCREEN_DESCRIPTOR_INDEX_SRV_POSW, incrementSize);
			device->CopyDescriptorsSimple(1, srvDest, tangentCPUAddress, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		//// Depth
		//{
		//	Ideal::D3D12DescriptorHandle depthHandle = DepthTexture->GetSRV();
		//	D3D12_CPU_DESCRIPTOR_HANDLE tangentCPUAddress = depthHandle.GetCpuHandle();
		//	CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(handle.GetCpuHandle(), SCREEN_DESCRIPTOR_INDEX_SRV_Depth, incrementSize);
		//	device->CopyDescriptorsSimple(1, srvDest, tangentCPUAddress, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//}
	}

	// Final Draw
	commandList->DrawIndexedInstanced(m_quadMesh->GetElementCount(), 1, 0, 0, 0);
}

void Ideal::IdealScreenQuad::Resize(std::shared_ptr<IdealRenderer> Renderer)
{

}
