#include "DebugMeshManager.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/Resource/IdealStaticMeshObject.h"

void Ideal::DebugMeshManager::Init(ComPtr<ID3D12Device> Device)
{
	CreateRootSignatureDebugMesh(Device);
	CreatePipelineStateDebugMesh(Device);

	CreateRootSignatureDebugLine(Device);
	CreatePipelineStateDebugLine(Device);
}

void Ideal::DebugMeshManager::SetDebugLineVB(std::shared_ptr<Ideal::D3D12VertexBuffer> VB)
{
	m_lineVB = VB;
}

void Ideal::DebugMeshManager::CreateRootSignatureDebugMesh(ComPtr<ID3D12Device> Device)
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::DebugMeshRootSignature::Slot::Count];
	ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Global].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Transform].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Color].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::DebugMeshRootSignature::Slot::Count];
	rootParameters[Ideal::DebugMeshRootSignature::Slot::CBV_Global].InitAsDescriptorTable(1, &ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Global]);
	rootParameters[Ideal::DebugMeshRootSignature::Slot::CBV_Transform].InitAsDescriptorTable(1, &ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Transform]);
	rootParameters[Ideal::DebugMeshRootSignature::Slot::CBV_Color].InitAsDescriptorTable(1, &ranges[Ideal::DebugMeshRootSignature::Slot::CBV_Color]);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC),
	};

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, staticSamplers, rootSignatureFlags);
	HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	Check(hr, L"Failed to serialize root signature in Debug Mesh Manager");

	hr = Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
	Check(hr, L"Failed to create RootSignature in Debug Mesh Manager");

	m_rootSignature->SetName(L"DebugMeshRootSignature");
}

void Ideal::DebugMeshManager::CreatePipelineStateDebugMesh(ComPtr<ID3D12Device> Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { BasicVertex::InputElements, BasicVertex::InputElementCount };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = m_vs->GetShaderByteCode();
	psoDesc.PS = m_ps->GetShaderByteCode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()));
	Check(hr, L"Faild to Create Pipeline State DEBUG MESH");
	return;
}

void Ideal::DebugMeshManager::CreateRootSignatureDebugLine(ComPtr<ID3D12Device> Device)
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::DebugLineRootSignature::Slot::Count];
	ranges[Ideal::DebugLineRootSignature::Slot::CBV_Global].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges[Ideal::DebugLineRootSignature::Slot::CBV_LineInfo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::DebugLineRootSignature::Slot::Count];
	rootParameters[Ideal::DebugLineRootSignature::Slot::CBV_Global].InitAsDescriptorTable(1, &ranges[Ideal::DebugLineRootSignature::Slot::CBV_Global]);
	rootParameters[Ideal::DebugLineRootSignature::Slot::CBV_LineInfo].InitAsDescriptorTable(1, &ranges[Ideal::DebugLineRootSignature::Slot::CBV_LineInfo]);
	//rootParameters[Ideal::DebugLineRootSignature::Slot::CBV_LineInfo].InitAsConstants(sizeof(), );// InitAsDescriptorTable(1, &ranges[Ideal::DebugLineRootSignature::Slot::CBV_LineInfo]);
	
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
	HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	Check(hr, L"Failed to serialize root signature in Debug line");

	hr = Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignatureDebugLine.GetAddressOf()));
	Check(hr, L"Failed to create RootSignature in Debug line");

	m_rootSignature->SetName(L"DebugLineRootSignature");
}

void Ideal::DebugMeshManager::CreatePipelineStateDebugLine(ComPtr<ID3D12Device> Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { GeometryVertex::InputElements, GeometryVertex::InputElementCount };
	psoDesc.pRootSignature = m_rootSignatureDebugLine.Get();
	psoDesc.VS = m_vsLine->GetShaderByteCode();
	psoDesc.PS = m_psLine->GetShaderByteCode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStateDebugLine.GetAddressOf()));
	Check(hr, L"Faild to Create Pipeline State DEBUG LINE");
	return;
}

void Ideal::DebugMeshManager::AddDebugMesh(std::shared_ptr<IdealStaticMeshObject> DebugMesh)
{
	m_meshes.push_back(DebugMesh);
}

void Ideal::DebugMeshManager::DeleteDebugMesh(std::shared_ptr<IdealStaticMeshObject> DebugMesh)
{
	std::weak_ptr w = DebugMesh;
	auto it = std::find_if(m_meshes.begin(), m_meshes.end(), [&w](const std::weak_ptr<Ideal::IdealStaticMeshObject>& p) {return p.lock() == w.lock(); });
	if (it != m_meshes.end())
	{
		std::swap(*it, m_meshes.back());
		m_meshes.pop_back();
	}
}

void Ideal::DebugMeshManager::SetVS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_vs = Shader;
}

void Ideal::DebugMeshManager::SetPS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_ps = Shader;
}

void Ideal::DebugMeshManager::SetVSLine(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_vsLine = Shader;
}

void Ideal::DebugMeshManager::SetPSLine(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_psLine = Shader;
}

void Ideal::DebugMeshManager::DrawDebugMeshes(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, CB_Global* CB_GlobalData)
{
	/// Bind To Shader
	{
		CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
		CommandList->SetPipelineState(m_pipelineState.Get());
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Global Data 
		auto cb0 = CBPool->Allocate(Device.Get(), sizeof(CB_Global));
		memcpy(cb0->SystemMemoryAddress, CB_GlobalData, sizeof(CB_Global));
		auto handle0 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle0.GetCpuHandle(), cb0->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::DebugMeshRootSignature::Slot::CBV_Global, handle0.GetGpuHandle());

		for (auto& m : m_meshes)
		{
			m.lock()->DebugDraw(Device, CommandList, DescriptorHeap, CBPool);
		}
	}

	{
		CommandList->SetGraphicsRootSignature(m_rootSignatureDebugLine.Get());
		CommandList->SetPipelineState(m_pipelineStateDebugLine.Get());
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		// Global Data
		auto cb0 = CBPool->Allocate(Device.Get(), sizeof(CB_Global));
		memcpy(cb0->SystemMemoryAddress, CB_GlobalData, sizeof(CB_Global));
		auto handle0 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle0.GetCpuHandle(), cb0->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::DebugLineRootSignature::Slot::CBV_Global, handle0.GetGpuHandle());

		const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_lineVB->GetView();
		
		CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		// TODO : Instancing
		for (auto& l : m_lines)
		{
			CB_DebugLine cbDebugLine;
			cbDebugLine.startPos = l.startPos;
			cbDebugLine.endPos = l.endPos;
			cbDebugLine.color = l.color;
			auto cb1 = CBPool->Allocate(Device.Get(), sizeof(CB_DebugLine));
			memcpy(cb1->SystemMemoryAddress, &cbDebugLine, sizeof(CB_DebugLine));
			auto handle1 = DescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle1.GetCpuHandle(), cb1->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetGraphicsRootDescriptorTable(Ideal::DebugLineRootSignature::Slot::CBV_LineInfo, handle1.GetGpuHandle());
		
			CommandList->DrawInstanced(2, 1, 0, 0);
		}
		//CommandList->DrawInstanced(2, static_cast<uint32>(m_lines.size()), 0, 0);
	}
}

void Ideal::DebugMeshManager::AddDebugLine(Vector3 start, Vector3 end, Color Color /*= Color(1,0,0)*/)
{
	m_lines.push_back({ start, 0, end, 0, Color });
}

//void Ideal::DebugMeshManager::DebugLineInstanceBake(ComPtr<ID3D12Device> Device, std::shared_ptr<Ideal::ResourceManager> ResourceManager)
//{
//	m_instanceBuffer = std::make_shared<Ideal::D3D12UploadBuffer>();
//	m_instanceBuffer->Create(Device.Get(), sizeof(CB_DebugLine) * 3000);
//	void* data = m_instanceBuffer->Map();
//	memcpy(data, m_lines.data(), sizeof(CB_DebugLine) * 3000);
//	m_instanceBuffer->UnMap();
//
//	// SRV로 만든다.
//}
