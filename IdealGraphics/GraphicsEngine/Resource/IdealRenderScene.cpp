#include "IdealRenderScene.h"

#include "GraphicsEngine/public/IdealRendererDefinitions.h"

#include "GraphicsEngine/D3D12/D3D12Renderer.h"

#include "GraphicsEngine/Resource/IdealStaticMeshObject.h"
#include "GraphicsEngine/Resource/IdealSkinnedMeshObject.h"
#include "GraphicsEngine/Resource/IdealScreenQuad.h"
#include "GraphicsEngine/Resource/Light/IdealDirectionalLight.h"
#include "GraphicsEngine/Resource/Light/IdealSpotLight.h"
#include "GraphicsEngine/Resource/Light/IdealPointLight.h"

#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/D3D12/D3D12PipelineStateObject.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
#include "GraphicsEngine/D3D12/D3D12Viewport.h"

#include "GraphicsEngine/ConstantBufferInfo.h"

#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"
Ideal::IdealRenderScene::IdealRenderScene()
	: m_width(0),
	m_height(0)
{

}

Ideal::IdealRenderScene::~IdealRenderScene()
{
	
}

void Ideal::IdealRenderScene::Init(std::shared_ptr<IdealRenderer> Renderer, std::shared_ptr<Ideal::DeferredDeleteManager> InDeferredDeleteManager)
{
	m_deferredDeleteManager = InDeferredDeleteManager;

	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);

	m_width = d3d12Renderer->GetWidth();
	m_height = d3d12Renderer->GetHeight();

	//---GBuffer SRV RTV---//
	CreateGBuffer(Renderer);
	//CreateDSV(Renderer);
	
	//---Screen Quad---//
	InitScreenQuad(Renderer);
	CreateScreenQuadRootSignature(d3d12Renderer->GetDevice().Get());
	CreateScreenQuadPSO(d3d12Renderer->GetDevice().Get());

#ifdef _DEBUG
	InitScreenQuadEditor(Renderer);
#endif

	//---Global CB---//
	CreateGlobalCB(Renderer);
	//---Light CB---//
	CreateLightCB(Renderer);
	//---Mesh---//
	CreateStaticMeshPSO(d3d12Renderer->GetDevice().Get());
	CreateSkinnedMeshPSO(d3d12Renderer->GetDevice().Get());

	// TEMP DEBUG //
	CreateDebugStaticMeshPSO(d3d12Renderer->GetDevice().Get());
}

void Ideal::IdealRenderScene::Draw(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	commandList->SetDescriptorHeaps(1, d3d12Renderer->GetMainDescriptorHeap()->GetDescriptorHeap().GetAddressOf());

	// CB
	UpdateGlobalCBData(Renderer);
	//UpdateLightCBData(Renderer);

	// Ver2 StaticMesh
	{
		commandList->SetPipelineState(m_staticMeshPSO->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_staticMeshRootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		BindDescriptorTable(Renderer);

		for (auto& m : m_staticMeshObjects)
		{
			if (m != nullptr)
			{
				m->Draw(d3d12Renderer);
			}
		}
	}
	// Ver2 SkinnedMesh
	{
		commandList->SetPipelineState(m_skinnedMeshPSO->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_skinnedMeshRootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		BindDescriptorTable(Renderer);

		for (auto& m : m_skinnedMeshObjects)
		{
			if (m!= nullptr)
			{
				m->Draw(Renderer);
			}
		}
	}
}

void Ideal::IdealRenderScene::Resize(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);

	m_width = d3d12Renderer->GetWidth();
	m_height = d3d12Renderer->GetHeight();

	CreateGBuffer(Renderer);
	InitScreenQuad(Renderer);
}

void Ideal::IdealRenderScene::Free()
{
	for (auto& t : m_gBuffers)
	{
		t.reset();
	}
	m_depthBuffer.reset();
	m_screenBuffer.reset();
}

void Ideal::IdealRenderScene::DrawGBuffer(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	// RenderTarget 설정
	TransitionGBufferToRTVandClear(Renderer);

	commandList->SetDescriptorHeaps(1, d3d12Renderer->GetMainDescriptorHeap()->GetDescriptorHeap().GetAddressOf());

	// CB
	AllocateFromDescriptorHeap(Renderer);
	UpdateGlobalCBData(Renderer);

	// Ver2 StaticMesh
	{
		commandList->SetPipelineState(m_staticMeshPSO->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_staticMeshRootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		BindDescriptorTable(Renderer);

		for (auto& m : m_staticMeshObjects)
		{
			if (m != nullptr)
			{
				m->Draw(d3d12Renderer);
			}
		}
	}
	// Ver2 SkinnedMesh
	{
		commandList->SetPipelineState(m_skinnedMeshPSO->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_skinnedMeshRootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		BindDescriptorTable(Renderer);

		for (auto& m : m_skinnedMeshObjects)
		{
			if (m != nullptr)
			{
				m->Draw(Renderer);
			}
		}
	}

	// TEMP WireFrame Mesh
	{
		commandList->SetPipelineState(m_debugStaticMeshPSO->GetPipelineState().Get());
		commandList->SetGraphicsRootSignature(m_staticMeshRootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		BindDescriptorTable(Renderer);

		for (auto& m : m_debugMeshObjects)
		{
			if (m != nullptr)
			{
				m->Draw(d3d12Renderer);
			}
		}
	}
}

void Ideal::IdealRenderScene::DrawScreen(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	UpdateLightCBData(Renderer);

	commandList->SetPipelineState(m_screenQuadPSO->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_screenQuadRootSignature.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	BindDescriptorTable(Renderer);

	TransitionGBufferToSRV(Renderer);

	m_fullScreenQuad->Draw(Renderer, m_gBuffers, nullptr);
}

void Ideal::IdealRenderScene::DrawScreenEditor(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	UpdateLightCBData(Renderer);

	commandList->SetPipelineState(m_screenQuadPSO->GetPipelineState().Get());
	commandList->SetGraphicsRootSignature(m_screenQuadRootSignature.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	BindDescriptorTable(Renderer);

	TransitionGBufferToSRV(Renderer);

	m_fullScreenQuad->Draw(Renderer, m_gBuffers, m_depthBuffer);
}

void Ideal::IdealRenderScene::AddObject(std::shared_ptr<Ideal::IMeshObject> MeshObject)
{
	auto meshType = MeshObject->GetMeshType();

	switch (meshType)
	{
		case Ideal::Static:
			m_staticMeshObjects.push_back(std::static_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject));
			break;
		case Ideal::Skinned:
			m_skinnedMeshObjects.push_back(std::static_pointer_cast<Ideal::IdealSkinnedMeshObject>(MeshObject));
			break;
		default:
			break;

	}
}

void Ideal::IdealRenderScene::AddDebugObject(std::shared_ptr<Ideal::IMeshObject> MeshObject)
{
	if (std::dynamic_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject) != nullptr)
	{
		m_debugMeshObjects.push_back(std::static_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject));
	}
}

void Ideal::IdealRenderScene::AddLight(std::shared_ptr<Ideal::ILight> Light)
{
	switch (Light->GetLightType())
	{
		case ELightType::None:
		{
			__debugbreak();
		}
		break;
		case ELightType::Directional:
		{
			// Directional Light는 그냥 바꿔준다.
			//m_directionalLight = std::static_pointer_cast<IdealDirectionalLight>(Light);
			m_directionalLights.push_back(std::static_pointer_cast<IdealDirectionalLight>(Light));
		}
		break;
		case ELightType::Spot:
		{
			m_spotLights.push_back(std::static_pointer_cast<IdealSpotLight>(Light));
		}
		break;
		case ELightType::Point:
		{
			m_pointLights.push_back(std::static_pointer_cast<IdealPointLight>(Light));
		}
		break;
		default:
		{

		}
		break;

	}
}

void Ideal::IdealRenderScene::DeleteObject(std::shared_ptr<Ideal::IMeshObject> MeshObject)
{
	if (MeshObject == nullptr)
		return;

	if (MeshObject->GetMeshType() == Ideal::Static)
	{
		auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject);

		auto it = std::find(m_staticMeshObjects.begin(), m_staticMeshObjects.end(), mesh);
		{
			*it = std::move(m_staticMeshObjects.back());
			m_deferredDeleteManager->AddMeshObjectToDeferredDelete(MeshObject);
			m_staticMeshObjects.pop_back();
		}
	}
	else if (MeshObject->GetMeshType() == Ideal::Skinned)
	{
		auto mesh = std::static_pointer_cast<Ideal::IdealSkinnedMeshObject>(MeshObject);

		auto it = std::find(m_skinnedMeshObjects.begin(), m_skinnedMeshObjects.end(), mesh);
		{
			*it = std::move(m_skinnedMeshObjects.back());
			m_deferredDeleteManager->AddMeshObjectToDeferredDelete(MeshObject);
			m_skinnedMeshObjects.pop_back();
		}
	}
}
void Ideal::IdealRenderScene::DeleteLight(std::shared_ptr<Ideal::ILight> Light)
{

}

void Ideal::IdealRenderScene::DeleteDebugObject(std::shared_ptr<Ideal::IMeshObject> MeshObject)
{
	if (MeshObject == nullptr)
		return;

	if (MeshObject->GetMeshType() == Ideal::Static)
	{
		auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject);

		auto it = std::find(m_debugMeshObjects.begin(), m_debugMeshObjects.end(), mesh);
		{
			*it = std::move(m_debugMeshObjects.back());
			m_deferredDeleteManager->AddMeshObjectToDeferredDelete(MeshObject);
			m_debugMeshObjects.pop_back();
		}
	}
}

void Ideal::IdealRenderScene::CreateStaticMeshPSO(ID3D12Device* Device)
{
	//std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Device);

	//-------------------Sampler--------------------//
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	//-------------------Range--------------------//
	CD3DX12_DESCRIPTOR_RANGE1 rangeGlobal[1];
	rangeGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Global

	CD3DX12_DESCRIPTOR_RANGE1 rangePerObj[1];
	rangePerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	// b1 : Transform

	CD3DX12_DESCRIPTOR_RANGE1 rangePerMesh[2];
	rangePerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2 : Material	// mesh마다 다른 material을 가지고 있으니 갈아 끼워야한다.
	rangePerMesh[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);	// t0 ~ t2 : Shader Resource View

	//-------------------Parameter--------------------//
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(_countof(rangeGlobal), rangeGlobal, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(_countof(rangePerObj), rangePerObj, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(_countof(rangePerMesh), rangePerMesh, D3D12_SHADER_VISIBILITY_ALL);


	//-------------------Signature--------------------//
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	Check(Device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(m_staticMeshRootSignature.GetAddressOf())
	));

	m_staticMeshPSO = std::make_shared<Ideal::D3D12PipelineStateObject>();
	m_staticMeshPSO->SetInputLayout(BasicVertex::InputElements, BasicVertex::InputElementCount);

	std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	vs->CompileFromFile(L"../Shaders/GBufferMesh2.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	m_staticMeshPSO->SetVertexShader(vs);

	std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	ps->CompileFromFile(L"../Shaders/GBufferMesh2.hlsl", nullptr, nullptr, "PS", "ps_5_0");
	m_staticMeshPSO->SetPixelShader(ps);

	m_staticMeshPSO->SetRootSignature(m_staticMeshRootSignature.Get());
	m_staticMeshPSO->SetRasterizerState();
	m_staticMeshPSO->SetBlendState();

	DXGI_FORMAT rtvFormat[m_gBufferNum] = {
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	m_staticMeshPSO->SetTargetFormat(m_gBufferNum, rtvFormat, dsvFormat);

	m_staticMeshPSO->Create(Device);
}

void Ideal::IdealRenderScene::CreateSkinnedMeshPSO(ID3D12Device* Device)
{
	//std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Device);

	//-------------------Sampler--------------------//
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	//-------------------Range--------------------//
	CD3DX12_DESCRIPTOR_RANGE1 rangeGlobal[1];
	rangeGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : global

	CD3DX12_DESCRIPTOR_RANGE1 rangePerObj[1];
	rangePerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1); // b1 : transform, b1 : bone

	CD3DX12_DESCRIPTOR_RANGE1 rangePerMesh[2];
	rangePerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);	// b3 : Material	// mesh마다 다른 material을 가지고 있으니 갈아 끼워야한다.
	rangePerMesh[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);	// t0 ~ t2 : Shader Resource View


	//-------------------Parameter--------------------//
	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(_countof(rangeGlobal), rangeGlobal, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(_countof(rangePerObj), rangePerObj, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(_countof(rangePerMesh), rangePerMesh, D3D12_SHADER_VISIBILITY_ALL);


	//-------------------Signature--------------------//
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	Check(Device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(m_skinnedMeshRootSignature.GetAddressOf())
	));

	m_skinnedMeshPSO = std::make_shared<Ideal::D3D12PipelineStateObject>();
	m_skinnedMeshPSO->SetInputLayout(SkinnedVertex::InputElements, SkinnedVertex::InputElementCount);

	std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	//vs->CompileFromFile(L"../Shaders/AnimationDefault.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	vs->CompileFromFile(L"../Shaders/AnimationGeometry2.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	m_skinnedMeshPSO->SetVertexShader(vs);

	std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	//ps->CompileFromFile(L"../Shaders/AnimationDefault.hlsl", nullptr, nullptr, "PS", "ps_5_0");
	ps->CompileFromFile(L"../Shaders/AnimationGeometry2.hlsl", nullptr, nullptr, "PS", "ps_5_0");
	m_skinnedMeshPSO->SetPixelShader(ps);

	m_skinnedMeshPSO->SetRootSignature(m_skinnedMeshRootSignature.Get());
	m_skinnedMeshPSO->SetRasterizerState();
	m_skinnedMeshPSO->SetBlendState();

	DXGI_FORMAT rtvFormat[m_gBufferNum] = {
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	m_skinnedMeshPSO->SetTargetFormat(m_gBufferNum, rtvFormat, dsvFormat);

	m_skinnedMeshPSO->Create(Device);
}

void Ideal::IdealRenderScene::CreateDebugStaticMeshPSO(ID3D12Device* Device)
{
	////-------------------Sampler--------------------//
	//CD3DX12_STATIC_SAMPLER_DESC sampler(
	//	0,
	//	D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP
	//);

	////-------------------Range--------------------//
	//CD3DX12_DESCRIPTOR_RANGE1 rangeGlobal[1];
	//rangeGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Global

	//CD3DX12_DESCRIPTOR_RANGE1 rangePerObj[1];
	//rangePerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	// b1 : Transform

	//CD3DX12_DESCRIPTOR_RANGE1 rangePerMesh[2];
	//rangePerMesh[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2 : Material	// mesh마다 다른 material을 가지고 있으니 갈아 끼워야한다.
	//rangePerMesh[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);	// t0 ~ t2 : Shader Resource View

	////-------------------Parameter--------------------//
	//CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	//rootParameters[0].InitAsDescriptorTable(_countof(rangeGlobal), rangeGlobal, D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[1].InitAsDescriptorTable(_countof(rangePerObj), rangePerObj, D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[2].InitAsDescriptorTable(_countof(rangePerMesh), rangePerMesh, D3D12_SHADER_VISIBILITY_ALL);


	////-------------------Signature--------------------//
	//D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	//ComPtr<ID3DBlob> signature;
	//ComPtr<ID3DBlob> error;
	//Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	//Check(Device->CreateRootSignature(
	//	0,
	//	signature->GetBufferPointer(),
	//	signature->GetBufferSize(),
	//	IID_PPV_ARGS(m_staticMeshRootSignature.GetAddressOf())
	//));

	m_debugStaticMeshPSO = std::make_shared<Ideal::D3D12PipelineStateObject>();
	m_debugStaticMeshPSO->SetInputLayout(BasicVertex::InputElements, BasicVertex::InputElementCount);

	std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	vs->CompileFromFile(L"../Shaders/GBufferMesh2.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	m_debugStaticMeshPSO->SetVertexShader(vs);

	std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	ps->CompileFromFile(L"../Shaders/GBufferMesh2.hlsl", nullptr, nullptr, "PS", "ps_5_0");
	m_debugStaticMeshPSO->SetPixelShader(ps);

	m_debugStaticMeshPSO->SetRootSignature(m_staticMeshRootSignature.Get());

	D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	m_debugStaticMeshPSO->SetRasterizerState(rasterizerDesc);
	m_debugStaticMeshPSO->SetBlendState();

	DXGI_FORMAT rtvFormat[m_gBufferNum] = {
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	m_debugStaticMeshPSO->SetTargetFormat(m_gBufferNum, rtvFormat, dsvFormat);

	m_debugStaticMeshPSO->Create(Device);
}

void Ideal::IdealRenderScene::AllocateFromDescriptorHeap(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	ComPtr<ID3D12Device> device = d3d12Renderer->GetDevice();
	std::shared_ptr<Ideal::D3D12DescriptorHeap> descriptorHeap = d3d12Renderer->GetMainDescriptorHeap();

	auto handle = descriptorHeap->Allocate(2);	// b0, b1
	m_cbGlobalHandle = handle;
}

void Ideal::IdealRenderScene::CreateGlobalCB(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	m_cbGlobal = std::make_shared<CB_Global>();
}

void Ideal::IdealRenderScene::UpdateGlobalCBData(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	ComPtr<ID3D12Device> device = d3d12Renderer->GetDevice();

	m_cbGlobal->View = d3d12Renderer->GetView().Transpose();
	m_cbGlobal->Proj = d3d12Renderer->GetProj().Transpose();
	m_cbGlobal->ViewProj = d3d12Renderer->GetViewProj().Transpose();
	m_cbGlobal->eyePos = d3d12Renderer->GetEyePos();

	std::shared_ptr<Ideal::D3D12DescriptorHeap> descriptorHeap = d3d12Renderer->GetMainDescriptorHeap();
	auto cb = d3d12Renderer->ConstantBufferAllocate(sizeof(CB_Global));
	if (!cb)
	{
		__debugbreak();
	}

	CB_Global* cbGlobal = (CB_Global*)cb->SystemMemAddr;
	//*cbGlobal = *(m_cbGlobal.get());
	*cbGlobal = *m_cbGlobal;

	uint32 incrementSize = d3d12Renderer->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Copy To Main Descriptor
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(m_cbGlobalHandle.GetCpuHandle(), GLOBAL_DESCRIPTOR_INDEX_CBV_GLOBAL, incrementSize);
	device->CopyDescriptorsSimple(1, cbvDest, cb->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Ideal::IdealRenderScene::BindDescriptorTable(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	commandList->SetGraphicsRootDescriptorTable(GLOBAL_DESCRIPTOR_TABLE_INDEX, m_cbGlobalHandle.GetGpuHandle());
}

void Ideal::IdealRenderScene::CreateLightCB(std::shared_ptr<IdealRenderer> Renderer)
{
	m_cbLightList = std::make_shared<CB_LightList>();
}

void Ideal::IdealRenderScene::UpdateLightCBData(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	ComPtr<ID3D12Device> device = d3d12Renderer->GetDevice();

	//----------------Directional Light-----------------//
	uint32 directionalLightNum = m_directionalLights.size();
	if (directionalLightNum > MAX_DIRECTIONAL_LIGHT_NUM)
	{
		directionalLightNum = MAX_DIRECTIONAL_LIGHT_NUM;
	}
	for (uint32 i = 0; i < directionalLightNum; ++i)
	{
		m_cbLightList->DirLights[i] = m_directionalLights[i]->GetDirectionalLightDesc();
	}
	m_cbLightList->DirLightNum = directionalLightNum;

	//----------------Spot Light-----------------//
	uint32 spotLightNum = m_spotLights.size();

	if (spotLightNum > MAX_SPOT_LIGHT_NUM)
	{
		spotLightNum = MAX_SPOT_LIGHT_NUM;
	}

	for (uint32 i = 0; i < spotLightNum; ++i)
	{
		m_cbLightList->SpotLights[i] = m_spotLights[i]->GetSpotLightDesc();
	}
	m_cbLightList->SpotLightNum = spotLightNum;

	//----------------Point Light-----------------//
	uint32 pointLightNum = m_pointLights.size();

	if (pointLightNum > MAX_POINT_LIGHT_NUM)
	{
		pointLightNum = MAX_POINT_LIGHT_NUM;
	}
	m_cbLightList->PointLightNum = pointLightNum;

	for (uint32 i = 0; i < pointLightNum; ++i)
	{
		m_cbLightList->PointLights[i] = m_pointLights[i]->GetPointLightDesc();
	}

	std::shared_ptr<Ideal::D3D12DescriptorHeap> descriptorHeap = d3d12Renderer->GetMainDescriptorHeap();
	auto cb = d3d12Renderer->ConstantBufferAllocate(sizeof(CB_LightList));
	if (!cb)
	{
		__debugbreak();
	}

	CB_LightList* cbLightList = (CB_LightList*)cb->SystemMemAddr;
	*cbLightList = *m_cbLightList;
	uint32 incrementSize = d3d12Renderer->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(m_cbGlobalHandle.GetCpuHandle(), GLOBAL_DESCRIPTOR_INDEX_CBV_LIGHTLIST, incrementSize);
	device->CopyDescriptorsSimple(1, cbvDest, cb->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Ideal::IdealRenderScene::CreateGBuffer(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	std::shared_ptr<Ideal::ResourceManager> resourceManager = d3d12Renderer->GetResourceManager();

	uint32 width = d3d12Renderer->GetWidth();
	uint32 height = d3d12Renderer->GetHeight();

	m_gBuffers.clear();

	Ideal::IdealTextureTypeFlag textureFlag = IDEAL_TEXTURE_NONE;
	textureFlag |= IDEAL_TEXTURE_RTV;
	textureFlag |= IDEAL_TEXTURE_SRV;

	{
		std::shared_ptr<Ideal::D3D12Texture> gBuffer = nullptr;
		resourceManager->CreateEmptyTexture2D(gBuffer, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, textureFlag, L"Gbuffer0");
		m_gBuffers.push_back(gBuffer);
	}
	{
		std::shared_ptr<Ideal::D3D12Texture> gBuffer = nullptr;
		resourceManager->CreateEmptyTexture2D(gBuffer, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, textureFlag, L"Gbuffer1");
		m_gBuffers.push_back(gBuffer);
	}
	{
		std::shared_ptr<Ideal::D3D12Texture> gBuffer = nullptr;
		resourceManager->CreateEmptyTexture2D(gBuffer, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, textureFlag, L"Gbuffer2");
		m_gBuffers.push_back(gBuffer);
	}
	{
		std::shared_ptr<Ideal::D3D12Texture> gBuffer = nullptr;
		resourceManager->CreateEmptyTexture2D(gBuffer, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, textureFlag, L"Gbuffer3");
		m_gBuffers.push_back(gBuffer);
	}

	//m_gBufferClearColors[0] = DirectX::Colors::AliceBlue;
	m_gBufferClearColors[0] = DirectX::Colors::AliceBlue;
	//m_gBufferClearColors[0] = DirectX::Colors::Black;
	m_gBufferClearColors[1] = DirectX::Colors::Black;
	m_gBufferClearColors[2] = DirectX::Colors::Black;	// h
	m_gBufferClearColors[3] = DirectX::Colors::Green;

}

void Ideal::IdealRenderScene::TransitionGBufferToRTVandClear(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	std::shared_ptr<Ideal::ResourceManager> resourceManager = d3d12Renderer->GetResourceManager();

	// 2024.05.17 이거 안해주면 rtv에 안그려지네?
	commandList->RSSetViewports(1, &(d3d12Renderer->GetViewport()->GetViewport()));
	commandList->RSSetScissorRects(1, &(d3d12Renderer->GetViewport()->GetScissorRect()));

	for (auto t : m_gBuffers)
	{
		CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
			t->GetResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		commandList->ResourceBarrier(1, &backBufferRenderTarget);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[4] =
	{
		m_gBuffers[0]->GetRTV().GetCpuHandle(),
		m_gBuffers[1]->GetRTV().GetCpuHandle(),
		m_gBuffers[2]->GetRTV().GetCpuHandle(),
		m_gBuffers[3]->GetRTV().GetCpuHandle()
	};

	commandList->OMSetRenderTargets(4, rtvHandle, FALSE, &d3d12Renderer->GetDSV());
	commandList->ClearDepthStencilView(d3d12Renderer->GetDSV(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// TODO : ClearRenderTarget
	for (uint32 i = 0; i < m_gBufferNum; ++i)
	{
		float c[4] = { 0.f, 0.f, 0.f, 1.f };
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = m_gBuffers[i]->GetResource()->GetDesc().Format;
		clearValue.Color[0] = m_gBufferClearColors[i][0];
		clearValue.Color[1] = m_gBufferClearColors[i][1];
		clearValue.Color[2] = m_gBufferClearColors[i][2];
		clearValue.Color[3] = m_gBufferClearColors[i][3];

		//commandList->ClearRenderTargetView(m_gBuffers[i]->GetRTV().GetCpuHandle(), m_gBufferClearColors[i], 0, nullptr);
		auto desc = m_gBuffers[i]->GetResource()->GetDesc();
		commandList->ClearRenderTargetView(m_gBuffers[i]->GetRTV().GetCpuHandle(), clearValue.Color, 0, nullptr);
	}
}

void Ideal::IdealRenderScene::TransitionGBufferToSRV(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();

	for (auto t : m_gBuffers)
	{
		CD3DX12_RESOURCE_BARRIER backBufferRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
			t->GetResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		commandList->ResourceBarrier(1, &backBufferRenderTarget);
	}
}

void Ideal::IdealRenderScene::CreateDSV(std::shared_ptr<IdealRenderer> Renderer)
{
	__debugbreak();
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12Renderer->GetCommandList();
	std::shared_ptr<Ideal::ResourceManager> resourceManager = d3d12Renderer->GetResourceManager();

	//-------DSV-------//
	//resourceManager->CreateTextureDSV(m_depthBuffer, m_width, m_height);

}

void Ideal::IdealRenderScene::InitScreenQuad(std::shared_ptr<IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	std::shared_ptr<Ideal::ResourceManager> resourceManager = d3d12Renderer->GetResourceManager();

	m_fullScreenQuad = std::make_shared<Ideal::IdealScreenQuad>();
	m_fullScreenQuad->Init(Renderer);

}

void Ideal::IdealRenderScene::CreateScreenQuadRootSignature(ID3D12Device* Device)
{
	//std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Device);

	//-------------------Sampler--------------------//
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	//-------------------Range--------------------//
	CD3DX12_DESCRIPTOR_RANGE1 rangeGlobal[1];
	rangeGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0 : global, b1 : lightList

	CD3DX12_DESCRIPTOR_RANGE1 rangeGBuffers[1];
	rangeGBuffers[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);	// t0 : Albedo, t1 : Normal, t2 : POSH, t3 : POSH (Temp) t4 : POSW, t5 : Depth


	//-------------------Parameter--------------------//
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(_countof(rangeGlobal), rangeGlobal, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(_countof(rangeGBuffers), rangeGBuffers, D3D12_SHADER_VISIBILITY_ALL);


	//-------------------Signature--------------------//
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	Check(Device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(m_screenQuadRootSignature.GetAddressOf())
	));
}

void Ideal::IdealRenderScene::CreateScreenQuadPSO(ID3D12Device* Device)
{
	//std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Device);

	m_screenQuadPSO = std::make_shared<Ideal::D3D12PipelineStateObject>();
	m_screenQuadPSO->SetInputLayout(QuadVertex::InputElements, QuadVertex::InputElementCount);

	std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	vs->CompileFromFile(L"../Shaders/FullScreenVS.hlsl", nullptr, nullptr, "main", "vs_5_0");
	m_screenQuadPSO->SetVertexShader(vs);

	// TODO : Light
	std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	ps->CompileFromFile(L"../Shaders/LightPassPS.hlsl", nullptr, nullptr, "main", "ps_5_0");
	m_screenQuadPSO->SetPixelShader(ps);

	m_screenQuadPSO->SetRootSignature(m_screenQuadRootSignature.Get());
	m_screenQuadPSO->SetRasterizerState();
	m_screenQuadPSO->SetBlendState();

	DXGI_FORMAT rtvFormat[1] = {
		DXGI_FORMAT_R8G8B8A8_UNORM
	};
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	m_screenQuadPSO->SetTargetFormat(1, rtvFormat, dsvFormat);

	m_screenQuadPSO->Create(Device);
}

void Ideal::IdealRenderScene::InitScreenQuadEditor(std::shared_ptr<Ideal::IdealRenderer> Renderer)
{
	std::shared_ptr<Ideal::D3D12Renderer> d3d12Renderer = std::static_pointer_cast<Ideal::D3D12Renderer>(Renderer);
	std::shared_ptr<Ideal::ResourceManager> resourceManager = d3d12Renderer->GetResourceManager();
	// main texture rtv // 2024.06.02
	m_screenBuffer = nullptr;
	//resourceManager->CreateRTV(m_screenBuffer, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, L"ScreenQuad");

	Ideal::IdealTextureTypeFlag flag = IDEAL_TEXTURE_RTV;
	resourceManager->CreateEmptyTexture2D(m_screenBuffer, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, flag, L"ScreenQuad");
}
