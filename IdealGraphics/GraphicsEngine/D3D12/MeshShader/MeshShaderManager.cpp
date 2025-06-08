#include "GraphicsEngine/D3D12/MeshShader/MeshShaderManager.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
#include "GraphicsEngine/Resource/IdealStaticMeshObject.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "Misc/MeshOptimizer/meshoptimizer.h"
#include "GraphicsEngine/D3D12/D3D12Descriptors.h"
#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"

inline uint32_t DivRoundUp(uint32_t num, uint32_t den) { return (num + den - 1) / den; }

Ideal::MeshShaderManager::MeshShaderManager()
{

}

void Ideal::MeshShaderManager::SetMesh(std::shared_ptr<Ideal::IMeshObject> Mesh, std::shared_ptr<Ideal::ResourceManager> ResourceManager)
{
	/////////////// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST////////////
	m_testMesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(Mesh);
	auto getMesh = m_testMesh->GetMeshByIndex(0);
	auto castedMesh = std::static_pointer_cast<Ideal::IdealMesh<BasicVertex>>(getMesh.lock());
	auto vertices = castedMesh->GetVerticesRef();
	auto indices = castedMesh->GetIndicesRef();

	// Meshlet Generation
	uint32 maxVertices = 64;
	uint32 maxTriangles = 128;


	const std::size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);

	std::vector<meshopt_Meshlet> meshoptMeshlets;
	std::vector<uint32> meshletVertexIndices;	// 정점 인덱스 배열
	std::vector<uint8> meshletTriangles;
	meshoptMeshlets.resize(maxMeshlets);
	meshletVertexIndices.resize(maxMeshlets * maxVertices);
	meshletTriangles.resize(maxMeshlets * maxVertices * 3);
	
	m_meshletCount = meshopt_buildMeshlets(
		meshoptMeshlets.data(),
		meshletVertexIndices.data(),
		meshletTriangles.data(),
		indices.data(),
		indices.size(),
		(const float*)(vertices.data()) + offsetof(BasicVertex, Position)/sizeof(float),
		vertices.size(),
		sizeof(BasicVertex),
		maxVertices,
		maxTriangles,
		0.f	// occlusion culling을 위해 필요하다고 함.
	);

	//meshopt_optimizeMeshlet(meshletVertexIndices[meshoptMeshlets.], meshletTriangles.data(), meshletTriangles.size(), meshletVertexIndices.size());



	const meshopt_Meshlet& last = meshoptMeshlets[m_meshletCount - 1];

	m_meshletBuffer = std::make_shared<Ideal::D3D12StructuredBuffer>();
	m_meshletVertexIndicesBuffer = std::make_shared<Ideal::D3D12StructuredBuffer>();
	m_meshletTrianglesBuffer = std::make_shared<Ideal::D3D12StructuredBuffer>();
	ResourceManager->CreateStructuredBuffer<meshopt_Meshlet>(m_meshletBuffer, meshoptMeshlets);
	ResourceManager->CreateStructuredBuffer<uint32>(m_meshletVertexIndicesBuffer, meshletVertexIndices);
	ResourceManager->CreateStructuredBuffer<uint8>(m_meshletTrianglesBuffer, meshletTriangles);
}

void Ideal::MeshShaderManager::Init(ComPtr<ID3D12Device5> Device)
{
	CreateRootSignature(Device);
	CreatePipelineState(Device);
}

void Ideal::MeshShaderManager::Draw(ComPtr<ID3D12Device5> Device, ComPtr<ID3D12GraphicsCommandList6> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap2> DescriptorHeap, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBAllocator, const CB_Global& CBData)
{
	CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
	CommandList->SetPipelineState(m_pipelineState.Get());
	/// 여기 할 것 // 2025.03.14 // Descriptor 연결, 위 SRV들 연결!!!
	{
		// Global Data 
		auto cb0 = CBAllocator->Allocate(Device.Get(), sizeof(CB_Global));
		memcpy(cb0->SystemMemoryAddress, &CBData, sizeof(CB_Global));
		auto handle = DescriptorHeap->Allocate(1);
		DeferredDeleteManager->AddDescriptorHandleToDeferredDelete(handle);
		Device->CopyDescriptorsSimple(1, handle.GetCPUDescriptorHandleStart(), cb0->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::MeshShaderRootSignature::Slot::CBV_Global, handle.GetGPUDescriptorHandleStart());

	}
	{
		auto getMesh = m_testMesh->GetMeshByIndex(0);
		auto castedMesh = std::static_pointer_cast<Ideal::IdealMesh<BasicVertex>>(getMesh.lock());
		auto vertexSRVHandle = castedMesh->GetVertexBuffer()->GetSRV2();
		auto handle = DescriptorHeap->Allocate(1);

		Device->CopyDescriptorsSimple(1, handle.GetCPUDescriptorHandleStart(), vertexSRVHandle.GetCPUDescriptorHandleStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::MeshShaderRootSignature::Slot::SRV_Vertices, handle.GetGPUDescriptorHandleStart());
		DeferredDeleteManager->AddDescriptorHandleToDeferredDelete(handle);
	}
	{
		auto handle = DescriptorHeap->Allocate(1);
		Device->CopyDescriptorsSimple(1, handle.GetCPUDescriptorHandleStart(), m_meshletBuffer->GetSRV2().GetCPUDescriptorHandleStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::MeshShaderRootSignature::Slot::SRV_Meshlet, handle.GetGPUDescriptorHandleStart());
		DeferredDeleteManager->AddDescriptorHandleToDeferredDelete(handle);
	}
	{
		auto handle = DescriptorHeap->Allocate(1);
		Device->CopyDescriptorsSimple(1, handle.GetCPUDescriptorHandleStart(), m_meshletVertexIndicesBuffer->GetSRV2().GetCPUDescriptorHandleStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::MeshShaderRootSignature::Slot::SRV_MeshletVertexIndices, handle.GetGPUDescriptorHandleStart());
		DeferredDeleteManager->AddDescriptorHandleToDeferredDelete(handle);
	}
	{
		auto handle = DescriptorHeap->Allocate(1);
		Device->CopyDescriptorsSimple(1, handle.GetCPUDescriptorHandleStart(), m_meshletTrianglesBuffer->GetSRV2().GetCPUDescriptorHandleStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::MeshShaderRootSignature::Slot::SRV_MeshletTriangle, handle.GetGPUDescriptorHandleStart());
		DeferredDeleteManager->AddDescriptorHandleToDeferredDelete(handle);
	}

	//CommandList->DispatchMesh(DivRoundUp(m_meshletCount, 32), 1, 1);
	CommandList->DispatchMesh(m_meshletCount, 1, 1);
}

void Ideal::MeshShaderManager::SetMS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_meshShader = Shader;
}

void Ideal::MeshShaderManager::SetPS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_pixelShader = Shader;
}

void Ideal::MeshShaderManager::CreateRootSignature(ComPtr<ID3D12Device5> Device)
{
	// Create Root Signature
	// Root Signature

	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::MeshShaderRootSignature::Slot::Count];
	CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::MeshShaderRootSignature::Slot::Count];
	
	ranges[Ideal::MeshShaderRootSignature::Slot::CBV_Global].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges[Ideal::MeshShaderRootSignature::Slot::SRV_Vertices].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[Ideal::MeshShaderRootSignature::Slot::SRV_Meshlet].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletTriangle].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletVertexIndices].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	rootParameters[Ideal::MeshShaderRootSignature::Slot::CBV_Global].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::CBV_Global], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[Ideal::MeshShaderRootSignature::Slot::SRV_Vertices].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::SRV_Vertices], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[Ideal::MeshShaderRootSignature::Slot::SRV_Meshlet].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::SRV_Meshlet], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletTriangle].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletTriangle], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletVertexIndices].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::SRV_MeshletVertexIndices], D3D12_SHADER_VISIBILITY_ALL);
	
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
	
	HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	Check(hr, L"Failed to serialize root signature Mesh Shader");
	
	hr = Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
	Check(hr, L"Failed To Create Mesh Shader RootSignature!");
}

void Ideal::MeshShaderManager::CreatePipelineState(ComPtr<ID3D12Device5> Device)
{
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.MS = m_meshShader->GetShaderByteCode();
	psoDesc.PS = m_pixelShader->GetShaderByteCode();
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.SampleDesc = DefaultSampleDesc();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	
	Check(Device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()))
		,L"Failed To Create Mesh Shader Pipeline State!");
}
