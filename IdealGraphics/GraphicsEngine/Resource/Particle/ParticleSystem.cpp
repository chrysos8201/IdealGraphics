#include "GraphicsEngine/Resource/Particle/ParticleSystem.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "IMesh.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/Resource/Particle/ParticleMaterial.h"
#include "GraphicsEngine/D3D12/D3D12PipelineStateObject.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"
#include "Misc/Utils/RandomValue.h"
#include "GraphicsEngine/Resource/IdealCamera.h"
#include <random>

Ideal::ParticleSystem::ParticleSystem()
{
	m_cbTransform.World = Matrix::Identity;
	m_cbTransform.WorldInvTranspose = Matrix::Identity;
}

Ideal::ParticleSystem::~ParticleSystem()
{

}

void Ideal::ParticleSystem::SetMaterial(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial)
{
	__debugbreak();
}

std::weak_ptr<Ideal::IParticleMaterial> Ideal::ParticleSystem::GetMaterial()
{
	return m_particleMaterial;
}

void Ideal::ParticleSystem::SetTransformMatrix(const Matrix& Transform)
{
	m_transform = Transform;
}

const DirectX::SimpleMath::Matrix& Ideal::ParticleSystem::GetTransformMatrix() const
{
	return m_transform;
}

void Ideal::ParticleSystem::SetActive(bool IsActive)
{
	m_isActive = IsActive;
	if (m_playOnWake)
	{
		Play();
	}
}

bool Ideal::ParticleSystem::GetActive()
{
	return m_isActive;
}

void Ideal::ParticleSystem::SetDeltaTime(float DT)
{
	m_deltaTime = DT;
}

void Ideal::ParticleSystem::Play()
{
	m_isPlaying = true;
	m_currentTime = 0;
	m_currentDurationTime = 0;

	// 파티클초기화
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::Resume()
{
	m_isPlaying = true;
}

void Ideal::ParticleSystem::Pause()
{
	m_isPlaying = false;
}

void Ideal::ParticleSystem::SetStopWhenFinished(bool StopWhenFinished)
{
	m_stopWhenFinished = StopWhenFinished;
}

void Ideal::ParticleSystem::SetPlayOnWake(bool PlayOnWake)
{
	m_playOnWake = PlayOnWake;
}

float Ideal::ParticleSystem::GetCurrentDurationTime()
{
	return m_currentDurationTime;
}

void Ideal::ParticleSystem::SetMaxParticles(unsigned int MaxParticles)
{
	m_maxParticles = MaxParticles;
	m_cbParticleSystem.MaxParticles = m_maxParticles;
}

void Ideal::ParticleSystem::Init(ComPtr<ID3D12Device> Device, ComPtr<ID3D12RootSignature> RootSignature, std::shared_ptr<Ideal::ParticleMaterial> ParticleMaterial)
{
	m_particleMaterial = ParticleMaterial;
	m_rootSignature = RootSignature;
	m_ps = m_particleMaterial.lock()->GetShader();
	//RENDER_MODE_MESH_CreatePipelineState(Device);
}

void Ideal::ParticleSystem::Free()
{
	m_ParticleStructuredBuffer->Free();
}

void Ideal::ParticleSystem::SetResourceManager(std::shared_ptr<Ideal::ResourceManager> ResourceManager)
{
	m_ResourceManger = ResourceManager;
}

void Ideal::ParticleSystem::SetDeferredDeleteManager(std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager)
{
	m_DeferredDeleteManager = DeferredDeleteManager;
}

void Ideal::ParticleSystem::DrawParticle(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, Vector3 CameraPos, std::shared_ptr<Ideal::IdealCamera> Camera)
{
	m_currentTime += m_deltaTime;
	m_cbParticleSystem.CurrentTime = m_currentTime;
	m_cbParticleSystem.CurrentTime = m_currentDurationTime;

	if (m_isPlaying)
	{
		m_currentDurationTime += (m_deltaTime * m_simulationSpeed);
	}

	if (m_currentDurationTime >= m_duration)
	{
		if (!m_isLoop || m_stopWhenFinished)
		{
			// 반복이 아닐때 그냥 나간다.
			// 또는 한 번만 재생을 원하면 나간다.
			return;
		}
		m_currentDurationTime = 0.f;
		m_RENDERM_MODE_BILLBOARD_isDirty = true;
	}

	if (m_currentDurationTime > m_startLifetime)
	{
		return;
	}


	if (m_isTextureSheetAnimation)
	{
		// Animation 업데이트
		UpdateAnimationUV();
	}

	// TODO : root signature deferred delete
	switch (m_Renderer_Mode)
	{
		case Ideal::ParticleMenu::ERendererMode::Billboard:
		{

			if (!m_RENDER_MODE_BILLBOARD_pipelineState)
			{
				RENDER_MODE_BILLBOARD_CreatePipelineState(Device);
			}
			DrawRenderBillboard(Device, CommandList, DescriptorHeap, CBPool);
		}
		break;
		case Ideal::ParticleMenu::ERendererMode::Mesh:
		{
			if (!m_RENDER_MODE_MESH_pipelineState)
			{
				RENDER_MODE_MESH_CreatePipelineState(Device);
			}
			CommandList->SetPipelineState(m_RENDER_MODE_MESH_pipelineState.Get());
			DrawRenderMesh(Device, CommandList, DescriptorHeap, CBPool, Camera);
		}
		break;
		default:
			break;
	}
}

void Ideal::ParticleSystem::SetMeshVS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_MESH_VS = Shader;
}

void Ideal::ParticleSystem::SetBillboardVS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_BILLBOARD_VS = Shader;
}

void Ideal::ParticleSystem::SetBillboardGS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_RENDER_MODE_BILLBOARD_GS = Shader;
}

void Ideal::ParticleSystem::SetParticleVertexBuffer(std::shared_ptr<Ideal::D3D12VertexBuffer> ParticleVertexBuffer)
{
	//m_particleVertexBuffer = ParticleVertexBuffer;
}

void Ideal::ParticleSystem::SetBillboardCalculateComputePipelineState(ComPtr<ID3D12PipelineState> PipelineState)
{
	m_RENDER_MODE_BILLBOARD_ComputePipelineState = PipelineState;
}

void Ideal::ParticleSystem::RENDER_MODE_MESH_CreatePipelineState(ComPtr<ID3D12Device> Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { ParticleMeshVertex::InputElements, ParticleMeshVertex::InputElementCount };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = m_RENDER_MODE_MESH_VS->GetShaderByteCode();
	psoDesc.PS = m_ps->GetShaderByteCode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	{
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// --- blending mode ---//
		Ideal::ParticleMaterialMenu::EBlendingMode BlendMode = m_particleMaterial.lock()->GetBlendingMode();
		switch (BlendMode)
		{
			case Ideal::ParticleMaterialMenu::EBlendingMode::Additive:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;	// one // zero 일경우 검은색으로 바뀌어간다.
			}
			break;
			case Ideal::ParticleMaterialMenu::EBlendingMode::Alpha:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;	// one // zero 일경우 검은색으로 바뀌어간다.
			}
			break;
			case Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA; // Alpha blending
			}
			break;
			default:
				break;
		}



		//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;	// one // zero 일경우 검은색으로 바뀌어간다.
		//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;	// one // zero 일경우 검은색으로 바뀌어간다.
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		psoDesc.BlendState = blendDesc;
	}
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	if (m_particleMaterial.lock()->GetWriteDepthBuffer() == false)
	{
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	if (m_particleMaterial.lock()->GetBackFaceCulling())
	{
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	}
	else
	{
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_RENDER_MODE_MESH_pipelineState.GetAddressOf()));
	Check(hr, L"Faild to Create Mesh Pipeline State");
	return;
}

void Ideal::ParticleSystem::RENDER_MODE_BILLBOARD_CreatePipelineState(ComPtr<ID3D12Device> Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { ParticleVertex::InputElements, ParticleVertex::InputElementCount };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = m_RENDER_MODE_BILLBOARD_VS->GetShaderByteCode();
	psoDesc.GS = m_RENDER_MODE_BILLBOARD_GS->GetShaderByteCode();
	psoDesc.PS = m_ps->GetShaderByteCode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	{
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		// --- blending mode ---//
		Ideal::ParticleMaterialMenu::EBlendingMode BlendMode = m_particleMaterial.lock()->GetBlendingMode();
		switch (BlendMode)
		{
			case Ideal::ParticleMaterialMenu::EBlendingMode::Additive:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;	// one // zero 일경우 검은색으로 바뀌어간다.
			}
			break;
			case Ideal::ParticleMaterialMenu::EBlendingMode::Alpha:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;	// one // zero 일경우 검은색으로 바뀌어간다.
			}
			break;
			case Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive:
			{
				blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA; // Alpha blending
			}
			break;
			default:
				break;
		}
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		psoDesc.BlendState = blendDesc;
	}
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	if (m_particleMaterial.lock()->GetWriteDepthBuffer() == false)
	{
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	if (m_particleMaterial.lock()->GetBackFaceCulling())
	{
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	}
	else
	{
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_RENDER_MODE_BILLBOARD_pipelineState.GetAddressOf()));
	Check(hr, L"Faild to Create Billboard Pipeline State");
	return;
}

void Ideal::ParticleSystem::SetStartColor(const DirectX::SimpleMath::Color& StartColor)
{
	m_startColor = StartColor;
}

DirectX::SimpleMath::Color& Ideal::ParticleSystem::GetStartColor()
{
	//return m_cbParticleSystem.StartColor;
	return m_startColor;
}

void Ideal::ParticleSystem::SetStartSize(float Size)
{
	m_startSize = Size;
}

void Ideal::ParticleSystem::SetStartLifetime(float Time)
{
	m_startLifetime = Time;
}

float Ideal::ParticleSystem::GetStartLifetime()
{
	return m_startLifetime;
}

void Ideal::ParticleSystem::SetSimulationSpeed(float Speed)
{
	m_simulationSpeed = Speed;
}

float Ideal::ParticleSystem::GetSimulationSpeed()
{
	return m_simulationSpeed;
}

void Ideal::ParticleSystem::SetDuration(float Time)
{
	m_duration = Time;
}

float Ideal::ParticleSystem::GetDuration()
{
	return m_duration;
}

void Ideal::ParticleSystem::SetLoop(bool Loop)
{
	m_isLoop = Loop;
}

bool Ideal::ParticleSystem::GetLoop()
{
	return m_isLoop;
}

void Ideal::ParticleSystem::SetRateOverTime(bool Active)
{
	m_isUseRateOverTime = Active;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetEmissionRateOverTime(float Count)
{
	m_emissionRateOverTime = Count;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetShapeMode(bool Active)
{
	m_isUseShapeMode = Active;
}

void Ideal::ParticleSystem::SetShape(const Ideal::ParticleMenu::EShape& Shape)
{
	m_ShapeMode_shape = Shape;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetRadius(float Radius)
{
	m_radius = Radius;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetRadiusThickness(float RadiusThickness)
{
	m_radiusThickness = RadiusThickness;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocityOverLifetime(bool Active)
{
	m_isUseVelocityOverLifetime = Active;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocityDirectionMode(const Ideal::ParticleMenu::EMode& Mode)
{
	m_velocityDirectionMode = Mode;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocityDirectionRandom(float Min, float Max)
{
	m_velocityRandomDirectionMin = Min;
	m_velocityRandomDirectionMax = Max;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocityDirectionConst(const DirectX::SimpleMath::Vector3& Direction)
{
	m_velocityConstDirection = Direction;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocitySpeedModifierMode(const Ideal::ParticleMenu::EMode& Mode)
{
	m_velocitySpeedModifierMode = Mode;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocitySpeedModifierRandom(float Min, float Max)
{
	m_velocityRandomSpeedMin = Min;
	m_velocityRandomSpeedMax = Max;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::SetVelocitySpeedModifierConst(float Speed)
{
	m_velocityConstSpeed = Speed;
	m_RENDERM_MODE_BILLBOARD_isDirty = true;
}

void Ideal::ParticleSystem::UpdateShape()
{
	if (m_RENDERM_MODE_BILLBOARD_isDirty == true)
	{
		m_RENDERM_MODE_BILLBOARD_isDirty = false;
		UpdateParticleVertexBufferAndStructuredBuffer();
	}
}

void Ideal::ParticleSystem::UpdateParticleVertexBufferAndStructuredBuffer()
{
	//-------Vertex Buffer Update-------//
	if (m_particleVertexBuffer)
	{
		m_DeferredDeleteManager.lock()->AddD3D12ResourceToDelete(m_particleVertexBuffer->GetResource());
	}
	else
	{
		m_particleVertexBuffer = std::make_shared<Ideal::D3D12VertexBuffer>();

	}

	std::vector<ParticleVertex> vertices;
	vertices.resize(m_maxParticles);
	for (uint32 i = 0; i < m_maxParticles; ++i)
	{
		vertices[i].Color = Vector4(1.f, 1.f, 0.2f, 1.f);
	}
	m_ResourceManger.lock()->CreateVertexBuffer<ParticleVertex>(m_particleVertexBuffer, vertices);


	//-------Structured Buffer Update-------//
	if (m_ParticleStructuredBuffer)
	{
		m_DeferredDeleteManager.lock()->AddD3D12ResourceToDelete(m_ParticleStructuredBuffer->GetResource());
	}
	else
	{
		m_ParticleStructuredBuffer = std::make_shared<Ideal::D3D12StructuredBuffer>();
	}

	std::vector<ComputeParticle> startPos;
	startPos.resize(m_maxParticles);
	CreateParticleStartInfo(startPos);


	m_ResourceManger.lock()->CreateStructuredBuffer<ComputeParticle>(m_ParticleStructuredBuffer, startPos);
}

void Ideal::ParticleSystem::SetColorOverLifetime(bool Active)
{
	m_isUseColorOverLifetime = Active;
}

Ideal::IGradient& Ideal::ParticleSystem::GetColorOverLifetimeGradientGraph()
{
	return m_ColorOverLifetimeGradientGraph;
}

void Ideal::ParticleSystem::UpdateColorOverLifetime()
{
	if (m_isUseColorOverLifetime)
	{
		// 현재 그래프의 색상을 불러온다.
		Color colorAtCurrentTime = m_ColorOverLifetimeGradientGraph.GetColorAtPosition(m_currentDurationTime / m_duration);
		m_cbParticleSystem.StartColor = colorAtCurrentTime;
	}
	else
	{
		m_cbParticleSystem.StartColor = m_startColor;
	}
}

void Ideal::ParticleSystem::SetSizeOverLifetime(bool Active)
{
	m_isSizeOverLifetime = Active;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetSizeOverLifetimeAxisX()
{
	return m_SizeOverLifetimeAxisX;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetSizeOverLifetimeAxisY()
{
	return m_SizeOverLifetimeAxisY;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetSizeOverLifetimeAxisZ()
{
	return m_SizeOverLifetimeAxisZ;
}

void Ideal::ParticleSystem::UpdateSizeOverLifetime()
{
	if (!m_isSizeOverLifetime)
		return;
}

void Ideal::ParticleSystem::SetRotationOverLifetime(bool Active)
{
	m_isRotationOverLifetime = Active;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetRotationOverLifetimeAxisX()
{
	return m_RotationOverLifetimeAxisX;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetRotationOverLifetimeAxisY()
{
	return m_RotationOverLifetimeAxisY;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetRotationOverLifetimeAxisZ()
{
	return m_RotationOverLifetimeAxisZ;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData1X()
{
	return m_CustomData1_X;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData1Y()
{
	return m_CustomData1_Y;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData1Z()
{
	return m_CustomData1_Z;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData1W()
{
	return m_CustomData1_W;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData2X()
{
	return m_CustomData2_X;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData2Y()
{
	return m_CustomData2_Y;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData2Z()
{
	return m_CustomData2_Z;
}

Ideal::IBezierCurve& Ideal::ParticleSystem::GetCustomData2W()
{
	return m_CustomData2_W;
}

void Ideal::ParticleSystem::SetTextureSheetAnimation(bool Active)
{
	m_isTextureSheetAnimation = Active;
}

void Ideal::ParticleSystem::SetTextureSheetAnimationTiles(const DirectX::SimpleMath::Vector2& Tiles)
{
	m_animationTiles = Tiles;
}

void Ideal::ParticleSystem::SetRenderMode(Ideal::ParticleMenu::ERendererMode ParticleRendererMode)
{
	if (m_Renderer_Mode != ParticleRendererMode)
	{
		m_Renderer_Mode = ParticleRendererMode;
	}
}

Ideal::ParticleMenu::ERendererMode Ideal::ParticleSystem::GetRenderMode()
{
	return m_Renderer_Mode;
}

void Ideal::ParticleSystem::SetRenderMesh(std::shared_ptr<Ideal::IMesh> ParticleRendererMesh)
{
	// TODO : 예외 처리 Particle Vertex가 아닐 경우
	m_Renderer_Mesh = std::static_pointer_cast<Ideal::IdealMesh<ParticleMeshVertex>>(ParticleRendererMesh);
}

void Ideal::ParticleSystem::SetRenderMaterial(std::shared_ptr<Ideal::IParticleMaterial> ParticleRendererMaterial)
{
	m_particleMaterial = std::static_pointer_cast<Ideal::ParticleMaterial>(ParticleRendererMaterial);
}

void Ideal::ParticleSystem::SetCustomData(Ideal::ParticleMenu::ECustomData CustomData, Ideal::ParticleMenu::ECustomDataParameter CustomDataParameter, Ideal::ParticleMenu::ERangeMode RangeMode, float CustomDataFloat, float CustomDataFloat2 /*= 0.f*/)
{
	float* Data = nullptr;
	if (CustomData == Ideal::ParticleMenu::ECustomData::CustomData_1)
	{
		Data = m_cbParticleSystem.CustomData1;
	}
	else
	{
		Data = m_cbParticleSystem.CustomData2;
	}

	uint32 parameter = static_cast<uint32>(CustomDataParameter);

	switch (RangeMode)
	{
		case Ideal::ParticleMenu::ERangeMode::Constant:
		{
			Data[parameter] = CustomDataFloat;
		}
		break;
		case Ideal::ParticleMenu::ERangeMode::Curve:
		{
			Data[parameter] = CustomDataFloat;
			// TODO: CURVE
		}
		break;
		case Ideal::ParticleMenu::ERangeMode::RandomBetweenConstants:
		{
			__debugbreak();
		}
		break;
		case Ideal::ParticleMenu::ERangeMode::RandomBetweenCurves:
		{
			__debugbreak();
		}
		break;
		default:
			break;

	}
}

void Ideal::ParticleSystem::DrawRenderMesh(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, std::shared_ptr<Ideal::IdealCamera> Camera)
{

	// Transform Data
	{
		// RotationOverLifetime
		Matrix cal = Matrix::Identity;
		
		Matrix ScaleMatrix = Matrix::CreateScale(m_startSize);
		cal *= Matrix::CreateScale(m_startSize);
		if (m_isSizeOverLifetime)
		{
			float x = m_SizeOverLifetimeAxisX.GetPoint(m_currentDurationTime).y;
			float y = m_SizeOverLifetimeAxisY.GetPoint(m_currentDurationTime).y;
			float z = m_SizeOverLifetimeAxisZ.GetPoint(m_currentDurationTime).y;
			Vector3 newSize(x, y, z);
			cal *= Matrix::CreateScale(newSize);
			ScaleMatrix *= Matrix::CreateScale(newSize);
		}

		if (m_isRotationOverLifetime)
		{
			float xRot = m_RotationOverLifetimeAxisX.GetPoint(m_currentDurationTime).y;
			auto matX = Matrix::CreateRotationX(xRot * DirectX::XM_PI);
			float yRot = m_RotationOverLifetimeAxisY.GetPoint(m_currentDurationTime).y;
			auto matY = Matrix::CreateRotationY(yRot * DirectX::XM_PI);
			float zRot = m_RotationOverLifetimeAxisZ.GetPoint(m_currentDurationTime).y;
			auto matZ = Matrix::CreateRotationZ(zRot * DirectX::XM_PI);
			cal *= matX;
			cal *= matY;
			cal *= matZ;
		}
		m_cbTransform.World = m_transform.Transpose() * cal;
		m_cbTransform.WorldInvTranspose = m_cbTransform.World.Invert();

		auto cb1 = CBPool->Allocate(Device.Get(), sizeof(CB_Transform));
		memcpy(cb1->SystemMemAddr, &m_cbTransform, sizeof(CB_Transform));
		auto handle1 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle1.GetCpuHandle(), cb1->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_Transform, handle1.GetGpuHandle());
	}

	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_Renderer_Mesh.lock()->GetVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = m_Renderer_Mesh.lock()->GetIndexBufferView();

	CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	CommandList->IASetIndexBuffer(&indexBufferView);

	// CB_ParticleSystem
	{
		UpdateCustomData();
		UpdateColorOverLifetime();
		//if (m_currentTime > 1.0) m_currentTime = 1.0;
		m_cbParticleSystem.Time = m_currentTime;
		m_cbParticleSystem.Time = m_currentDurationTime;
		auto cb2 = CBPool->Allocate(Device.Get(), sizeof(CB_ParticleSystem));
		memcpy(cb2->SystemMemAddr, &m_cbParticleSystem, sizeof(CB_ParticleSystem));
		auto handle2 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle2.GetCpuHandle(), cb2->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData, handle2.GetGpuHandle());
	}
	// SRV
	if (m_particleMaterial.lock())
	{
		// Texture0
		{
			auto texture = m_particleMaterial.lock()->GetTexture0().lock();
			if (texture)
			{
				auto handle3 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle3.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture0, handle3.GetGpuHandle());
			}
		}
		// Texture1
		{
			auto texture = m_particleMaterial.lock()->GetTexture1().lock();
			if (texture)
			{
				auto handle4 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle4.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture1, handle4.GetGpuHandle());
			}
		}
		// Texture2
		{
			auto texture = m_particleMaterial.lock()->GetTexture2().lock();
			if (texture)
			{
				auto handle5 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle5.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture2, handle5.GetGpuHandle());
			}
		}
	}
	CommandList->DrawIndexedInstanced(m_Renderer_Mesh.lock()->GetElementCount(), 1, 0, 0, 0);
}

void Ideal::ParticleSystem::DrawRenderBillboard(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool)
{

	m_ParticleStructuredBuffer->TransitionToSRV(CommandList.Get());

	if (m_isSizeOverLifetime)
	{
		if (m_currentDurationTime <= 0.05f)
			int a = 3;
		float x = m_SizeOverLifetimeAxisX.GetPoint(m_currentDurationTime).y;
		float y = m_SizeOverLifetimeAxisY.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.ParticleSize.x = x;
		m_cbParticleSystem.ParticleSize.y = y;
	}
	else
	{
		m_cbParticleSystem.ParticleSize.x = 1;
		m_cbParticleSystem.ParticleSize.y = 1;
	}
	// CB_ParticleSystem
	{
		// Transform
		m_cbTransform.World = m_transform.Transpose();
		m_cbTransform.WorldInvTranspose = m_cbTransform.World.Invert();
		auto cb1 = CBPool->Allocate(Device.Get(), sizeof(CB_Transform));
		memcpy(cb1->SystemMemAddr, &m_cbTransform, sizeof(CB_Transform));
		auto handle1 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle1.GetCpuHandle(), cb1->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_Transform, handle1.GetGpuHandle());

		// CustomData
		UpdateCustomData();

		// Color
		UpdateColorOverLifetime();

		// ParticleData
		m_cbParticleSystem.Time = m_currentTime;
		m_cbParticleSystem.DeltaTime = m_deltaTime;
		auto cb2 = CBPool->Allocate(Device.Get(), sizeof(CB_ParticleSystem));
		memcpy(cb2->SystemMemAddr, &m_cbParticleSystem, sizeof(CB_ParticleSystem));
		auto handle2 = DescriptorHeap->Allocate();
		Device->CopyDescriptorsSimple(1, handle2.GetCpuHandle(), cb2->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData, handle2.GetGpuHandle());
	}
	// SRV
	if (m_particleMaterial.lock())
	{
		// StructuredBuffer PositionBuffer
		{
			auto handle = DescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), m_ParticleStructuredBuffer->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticlePosBuffer, handle.GetGpuHandle());
		}

		// Texture0
		{
			auto texture = m_particleMaterial.lock()->GetTexture0().lock();
			if (texture)
			{
				auto handle3 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle3.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture0, handle3.GetGpuHandle());
			}
		}
		// Texture1
		{
			auto texture = m_particleMaterial.lock()->GetTexture1().lock();
			if (texture)
			{
				auto handle4 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle4.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture1, handle4.GetGpuHandle());
			}
		}
		// Texture2
		{
			auto texture = m_particleMaterial.lock()->GetTexture2().lock();
			if (texture)
			{
				auto handle5 = DescriptorHeap->Allocate();
				Device->CopyDescriptorsSimple(1, handle5.GetCpuHandle(), texture->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::SRV_ParticleTexture2, handle5.GetGpuHandle());
			}
		}
	}
	CommandList->SetPipelineState(m_RENDER_MODE_BILLBOARD_pipelineState.Get());
	CommandList->IASetVertexBuffers(0, 1, &m_particleVertexBuffer->GetView());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	// TODO: 임시 100개
	CommandList->DrawInstanced(m_maxParticles, 1, 0, 0);
}

void Ideal::ParticleSystem::ComputeRenderBillboard(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool)
{
	// 먼저 컴퓨트 셰이더로 위치 계산을 하겠다. 만약 사용될 버퍼가 없으면 만든다.
	UpdateShape();
	m_ParticleStructuredBuffer->TransitionToUAV(CommandList.Get());
	{
		//CommandList->SetComputeRootSignature(m_RENDER_MODE_BILLBOARD_ComputePipelineState.Get())
		CommandList->SetPipelineState(m_RENDER_MODE_BILLBOARD_ComputePipelineState.Get());

		{
			auto handle = DescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), m_ParticleStructuredBuffer->GetUAV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetComputeRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::UAV_ParticlePosBuffer, handle.GetGpuHandle());
		}
		{
			// ParticleData
			//m_cbParticleSystem.Time = m_currentTime;
			m_cbParticleSystem.Time = m_currentDurationTime;
			m_cbParticleSystem.DeltaTime = m_deltaTime;
			auto cb2 = CBPool->Allocate(Device.Get(), sizeof(CB_ParticleSystem));
			memcpy(cb2->SystemMemAddr, &m_cbParticleSystem, sizeof(CB_ParticleSystem));
			auto handle = DescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), cb2->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetComputeRootDescriptorTable(Ideal::ParticleSystemRootSignature::Slot::CBV_ParticleSystemData, handle.GetGpuHandle());
		}

		// TODO : Dispatch
		const uint32 threadPerGroup = 256;
		uint32 threadGroupX = (m_maxParticles + threadPerGroup - 1) / threadPerGroup;
		CommandList->Dispatch(threadGroupX, 1, 1);
	}
}

void Ideal::ParticleSystem::UpdateCustomData()
{
	{
		float customX = m_CustomData1_X.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData1[0] = customX;
		float customY = m_CustomData1_Y.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData1[1] = customY;
		float customZ = m_CustomData1_Z.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData1[2] = customZ;
		float customW = m_CustomData1_W.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData1[3] = customW;
	}
	{
		float customX = m_CustomData2_X.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData2[0] = customX;
		float customY = m_CustomData2_Y.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData2[1] = customY;
		float customZ = m_CustomData2_Z.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData2[2] = customZ;
		float customW = m_CustomData2_W.GetPoint(m_currentDurationTime).y;
		m_cbParticleSystem.CustomData2[3] = customW;
	}
}

void Ideal::ParticleSystem::UpdateLifeTime()
{
	if (m_currentTime > m_startLifetime)
	{

	}
}

void Ideal::ParticleSystem::CreateParticleStartInfo(std::vector<ComputeParticle>& Vertices)
{
	switch (m_ShapeMode_shape)
	{
		case Ideal::ParticleMenu::EShape::Circle:
		{
			RandomValue randManager;
			float radius = m_radius;
			float radiusThickness = m_radiusThickness * m_radius;
			float pi = 3.1415926535f;

			std::vector<uint32> EmissionRateOverTimeCount;
			uint32 currentRateOverTime_SecondsIndex = 0;	// RateOverTimeCount에 현재 시간 인덱스를 가리키는 인덱스 변수
			uint32 currentRateOverTime_SecondsCount = 0;	// RateOverTimeCount가 가리키는 인덱스의 현재 만든 개수

			for (uint32 i = 0; i < m_maxParticles; ++i)
			{
				Vertices[i].Position = Vector4(i, 0, 0, 1);
				Vertices[i].Direction = Vector3(0, 0, 1);
				Vertices[i].Speed = 1.f;

				// Temp : Circle
				float innerRadius = radius - radiusThickness;
				float theta = randManager.nextFloat(0.f, 2.f * pi);
				float randRadius = sqrt(randManager.nextFloat(innerRadius * innerRadius, radius * radius));
				float x = randRadius * cos(theta);
				float y = randRadius * sin(theta);
				//Vertices[i].Position = Vector4(x, y, 0, 1);

				Matrix local = Matrix::CreateTranslation(Vector3(x, y, 0));
				Matrix world = local * m_transform;
				Vertices[i].Position = Vector4(world._41, world._42, world._43, 1.f);
				if (m_isUseVelocityOverLifetime == true)
				{
					//---Direction---//
					if (m_velocityDirectionMode == Ideal::ParticleMenu::EMode::Random)
					{
						float d0 = randManager.nextFloat(m_velocityRandomDirectionMin, m_velocityRandomDirectionMax);
						float d1 = randManager.nextFloat(m_velocityRandomDirectionMin, m_velocityRandomDirectionMax);
						float d2 = randManager.nextFloat(m_velocityRandomDirectionMin, m_velocityRandomDirectionMax);
						Vector3 dir = Vector3(d0, d1, d2);
						dir.Normalize();
						Vertices[i].Direction = dir;
					}
					else
					{
						Vector3 dir = m_velocityConstDirection;
						dir.Normalize();
						Vertices[i].Direction = dir;
					}

					//---Speed---//
					if (m_velocitySpeedModifierMode == Ideal::ParticleMenu::EMode::Random)
					{
						float speed = randManager.nextFloat(m_velocityRandomSpeedMin, m_velocityRandomSpeedMax);
						Vertices[i].Speed = speed;
					}
					else
					{
						Vertices[i].Speed = m_velocityConstSpeed;
					}

					//---Rotation---//
					float angle = randManager.nextFloat(0, 3.141592f);
					Vertices[i].RotationAngle = angle;
				}
				//----Delay Time----//
				if (m_isUseRateOverTime)
				{
					float delayTime = randManager.nextFloat(currentRateOverTime_SecondsIndex, currentRateOverTime_SecondsIndex + 1);
					Vertices[i].DelayTime = delayTime;
					currentRateOverTime_SecondsCount++;
					if (currentRateOverTime_SecondsCount > m_emissionRateOverTime)
					{
						currentRateOverTime_SecondsCount = 0;
						currentRateOverTime_SecondsIndex++;
					}
				}
			}
		}
		break;
		default:
			break;

	}
}

void Ideal::ParticleSystem::UpdateAnimationUV()
{
	uint32 gridX = m_animationTiles.x;
	uint32 gridY = m_animationTiles.y;

	// 총 애니메이션 프레임 개수
	uint32 totalFrames = gridX * gridY;

	float frameDuration = m_startLifetime / static_cast<float>(totalFrames);

	// 현재 프레임 계산
	uint32 currentFrame = static_cast<uint32>(m_currentDurationTime / frameDuration);

	// 경계를 초과하는 경우 마지막 프레임으로 고정
	currentFrame = std::min<uint32>(currentFrame, totalFrames - 1);

	// 현재 프레임의 행과 열 계산
	float FrameRow = currentFrame / gridX;
	float FrameCol = currentFrame % gridX;

	// UV 계산
	float uStart = FrameCol / static_cast<float>(gridX);
	float vStart = FrameRow / static_cast<float>(gridY);
	float uEnd = (FrameCol + 1) / static_cast<float>(gridX);
	float vEnd = (FrameRow + 1) / static_cast<float>(gridY);

	// UV Offset과 UV Scale 계산
	m_cbParticleSystem.AnimationUV_Offset = Vector2(uStart, vStart);
	m_cbParticleSystem.AnimationUV_Scale = Vector2(uEnd - uStart, vEnd - vStart);
}
