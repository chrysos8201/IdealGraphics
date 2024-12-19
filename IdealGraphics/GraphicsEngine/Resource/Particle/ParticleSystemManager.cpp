#include "ParticleSystemManager.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/Resource/Particle/ParticleSystem.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"

Ideal::ParticleSystemManager::ParticleSystemManager()
{

}

Ideal::ParticleSystemManager::~ParticleSystemManager()
{

}

void Ideal::ParticleSystemManager::Init(ComPtr<ID3D12Device> Device)
{
	CreateRootSignature(Device);
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> Ideal::ParticleSystemManager::GetRootSignature()
{
	return m_rootSignature;
}

std::shared_ptr<Ideal::D3D12Shader> Ideal::ParticleSystemManager::GetMeshVS()
{
	return m_RENDER_MODE_MESH_VS;
}

std::shared_ptr<Ideal::D3D12Shader> Ideal::ParticleSystemManager::GetBillboardVS()
{
	return m_RENDER_MODE_BILLBOARD_VS;
}

std::shared_ptr<Ideal::D3D12Shader> Ideal::ParticleSystemManager::GetBillboardGS()
{
	return m_RENDER_MODE_BILLBOARD_GS;
}

void Ideal::ParticleSystemManager::CreateRootSignature(ComPtr<ID3D12Device> Device)
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::ParticleSystemRootSignature::Slot::Count];
	ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_Global].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_Transform].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticlePosBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	ranges[Ideal::ParticleSystemRootSignature::Slot::UAV_ParticlePosBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);	// u0 : Particle Texture Pos Read Write 
	
	CD3DX12_ROOT_PARAMETER1	rootParameters[Ideal::ParticleSystemRootSignature::Slot::Count];
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::CBV_Global].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_Global]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::CBV_Transform].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_Transform]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticlePosBuffer].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticlePosBuffer], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture0].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture0]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture1].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture1]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture2].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture2]);
	rootParameters[Ideal::ParticleSystemRootSignature::Slot::UAV_ParticlePosBuffer].InitAsDescriptorTable(1, &ranges[Ideal::ParticleSystemRootSignature::Slot::UAV_ParticlePosBuffer]);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC),
		CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_ANISOTROPIC
		, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		)
	};

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 2, staticSamplers, rootSignatureFlags);
	HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	Check(hr, L"Failed to serialize root signature in particle system manager");

	hr = Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
	Check(hr, L"Failed to create RootSignature in particle system manager");

	m_rootSignature->SetName(L"ParticleSystemRootSignature");
}

void Ideal::ParticleSystemManager::CreateCSPipelineState(ComPtr<ID3D12Device> Device)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc = {};
	computePipelineDesc.pRootSignature = m_rootSignature.Get();
	CD3DX12_SHADER_BYTECODE shader(m_RENDER_MODE_BILLBOARD_CS->GetBufferPointer(), m_RENDER_MODE_BILLBOARD_CS->GetSize());
	computePipelineDesc.CS = shader;
	Check(
		Device->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&m_RENDER_MODE_BILLBOARD_Compute_PipelineState))
		, L"Failed to create Particle Billboard Compute Pipeline State"
	);
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Ideal::ParticleSystemManager::GetParticleComputePipelineState()
{
	return m_RENDER_MODE_BILLBOARD_Compute_PipelineState;
}

void Ideal::ParticleSystemManager::SetMeshVS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_MESH_VS = Shader;
}

void Ideal::ParticleSystemManager::SetBillboardVS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_BILLBOARD_VS = Shader;
}

void Ideal::ParticleSystemManager::SetBillboardGS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_BILLBOARD_GS = Shader;
}

void Ideal::ParticleSystemManager::SetBillboardCSAndCreatePipelineState(std::shared_ptr<Ideal::D3D12Shader> Shader, ComPtr<ID3D12Device> Device)
{
	m_RENDER_MODE_BILLBOARD_CS = Shader;
	CreateCSPipelineState(Device);
}

void Ideal::ParticleSystemManager::SetDefaultParticleVertexBuffer(std::shared_ptr<Ideal::D3D12VertexBuffer> ParticleVertexBuffer)
{
	m_particleVertexBuffer = ParticleVertexBuffer;
}

std::shared_ptr<Ideal::D3D12VertexBuffer> Ideal::ParticleSystemManager::GetParticleVertexBuffer()
{
	return m_particleVertexBuffer;
}

void Ideal::ParticleSystemManager::AddParticleSystemNoTransparency(std::shared_ptr<Ideal::ParticleSystem> ParticleSystem)
{
	m_particlesNoTransparency.push_back(ParticleSystem);
}

void Ideal::ParticleSystemManager::AddParticleSystem(std::shared_ptr<Ideal::ParticleSystem> ParticleSystem)
{
	m_particles.push_back(ParticleSystem);
}

void Ideal::ParticleSystemManager::DeleteParticleSystem(std::shared_ptr<Ideal::ParticleSystem>& ParticleSystem)
{
	auto it = std::find_if(m_particles.begin(), m_particles.end(), [&ParticleSystem](const std::shared_ptr<Ideal::ParticleSystem>& p) {return p == ParticleSystem; });
	if (it != m_particles.end()) 
	{
		std::swap(*it, m_particles.back());
		m_particles.pop_back();
		__debugbreak(); // 1023. deferred delete manager에게 넘겨준 다음 Free를 실행하도록 합시다.
	}
}

void Ideal::ParticleSystemManager::DrawParticles(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, CB_Global* CB_GlobalData, std::shared_ptr<Ideal::IdealCamera> Camera)
{
	// 먼저 위치 계산부터 하겠음
	CommandList->SetComputeRootSignature(m_rootSignature.Get());
	for (auto& p : m_particles)
	{
		if (p->GetActive())
		{
			if (p->GetRenderMode() == Ideal::ParticleMenu::ERendererMode::Billboard)
			{
				p->ComputeRenderBillboard(Device, CommandList, DescriptorHeap, CBPool);
			}
		}
	}
	for (auto& p : m_particlesNoTransparency)
	{
		if (p->GetActive())
		{
			if (p->GetRenderMode() == Ideal::ParticleMenu::ERendererMode::Billboard)
			{
				p->ComputeRenderBillboard(Device, CommandList, DescriptorHeap, CBPool);
			}
		}
	}

	CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
	/// Bind To Shader
	{
		// Global Data 
		auto cb0 = CBPool->Allocate(Device.Get(), sizeof(CB_Global));
		memcpy(cb0->SystemMemoryAddress, CB_GlobalData, sizeof(CB_Global));
		auto handle0 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle0.GetCpuHandle(), cb0->CpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_Global, handle0.GetGpuHandle());
	}
	for (auto& p : m_particlesNoTransparency)
	{
		// TODO: DRAW
		if (p->GetActive())
		{
			p->DrawParticle(Device, CommandList, DescriptorHeap, CBPool, CB_GlobalData->eyePos, Camera);
		}
	}
	for (auto& p : m_particles)
	{
		// TODO: DRAW
		if (p->GetActive())
		{
			p->DrawParticle(Device, CommandList, DescriptorHeap, CBPool, CB_GlobalData->eyePos, Camera);
		}
	}
}