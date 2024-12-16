#include "D3D12RayTracingRenderer.h"

#include <DirectXColors.h>
#include "Misc/Utils/PIX.h"
#include "Misc/Assimp/AssimpConverter.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/Raytracing/D3D12RaytracingResources.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12PipelineStateObject.h"
#include "GraphicsEngine/D3D12/D3D12RootSignature.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12Viewport.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/D3D12SRV.h"
#include "GraphicsEngine/D3D12/D3D12UAV.h"
#include "GraphicsEngine/D3D12/D3D12UploadBufferPool.h"
#include "GraphicsEngine/D3D12/Raytracing/DXRAccelerationStructure.h"
#include "GraphicsEngine/D3D12/Raytracing/DXRAccelerationStructureManager.h"
#include "GraphicsEngine/D3D12/Raytracing/RaytracingManager.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorManager.h"
#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"

#include "GraphicsEngine/Resource/Light/IdealDirectionalLight.h"
#include "GraphicsEngine/Resource/Light/IdealSpotLight.h"
#include "GraphicsEngine/Resource/Light/IdealPointLight.h"

#include "GraphicsEngine/Resource/ShaderManager.h"
#include "GraphicsEngine/Resource/IdealCamera.h"
#include "GraphicsEngine/Resource/IdealStaticMeshObject.h"
#include "GraphicsEngine/Resource/IdealAnimation.h"
#include "GraphicsEngine/Resource/IdealSkinnedMeshObject.h"
#include "GraphicsEngine/Resource/IdealScreenQuad.h"
#include "GraphicsEngine/Resource/IdealRenderScene.h"
#include "GraphicsEngine/Resource/UI/IdealCanvas.h"
#include "GraphicsEngine/Resource/UI/IdealText.h"

#include "GraphicsEngine/Resource/Particle/ParticleSystemManager.h"
#include "GraphicsEngine/Resource/Particle/ParticleSystem.h"
#include "GraphicsEngine/Resource/Particle/ParticleMaterial.h"

#include "GraphicsEngine/Resource/DebugMesh/DebugMeshManager.h"

#include "GraphicsEngine/Resource/Light/IdealDirectionalLight.h"
#include "GraphicsEngine/Resource/Light/IdealSpotLight.h"
#include "GraphicsEngine/Resource/Light/IdealPointLight.h"

#include "GraphicsEngine/Resource/IdealMaterial.h"
#include "GraphicsEngine/Resource/UI/IdealSprite.h"

#include "GraphicsEngine/D3D12/TestShader.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"

#include "GraphicsEngine/D2D/D2DTextManager.h"

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)


#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <istream>
#include <fstream>
#include <vector>
// Pretty-print a state object tree.
inline void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
	std::wstringstream wstr;
	wstr << L"\n";
	wstr << L"--------------------------------------------------------------------\n";
	wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

	auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
		{
			std::wostringstream woss;
			for (UINT i = 0; i < numExports; i++)
			{
				woss << L"|";
				if (depth > 0)
				{
					for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
				}
				woss << L" [" << i << L"]: ";
				if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
				woss << exports[i].Name << L"\n";
			}
			return woss.str();
		};

	for (UINT i = 0; i < desc->NumSubobjects; i++)
	{
		wstr << L"| [" << i << L"]: ";
		switch (desc->pSubobjects[i].Type)
		{
			case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
				wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
				break;
			case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
				wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
				break;
			case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
				wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
				break;
			case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
			{
				wstr << L"DXIL Library 0x";
				auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
				wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
				wstr << ExportTree(1, lib->NumExports, lib->pExports);
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
			{
				wstr << L"Existing Library 0x";
				auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
				wstr << collection->pExistingCollection << L"\n";
				wstr << ExportTree(1, collection->NumExports, collection->pExports);
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
			{
				wstr << L"Subobject to Exports Association (Subobject [";
				auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
				UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
				wstr << index << L"])\n";
				for (UINT j = 0; j < association->NumExports; j++)
				{
					wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
				}
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
			{
				wstr << L"DXIL Subobjects to Exports Association (";
				auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
				wstr << association->SubobjectToAssociate << L")\n";
				for (UINT j = 0; j < association->NumExports; j++)
				{
					wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
				}
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
			{
				wstr << L"Raytracing Shader Config\n";
				auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
				wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
				wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
			{
				wstr << L"Raytracing Pipeline Config\n";
				auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
				wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
				break;
			}
			case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
			{
				wstr << L"Hit Group (";
				auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
				wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
				wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
				wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
				wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
				break;
			}
		}
		wstr << L"|--------------------------------------------------------------------\n";
	}
	wstr << L"\n";
	OutputDebugStringW(wstr.str().c_str());
}

inline void AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	Check(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
	void* pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);
}

inline void AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Check(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
}

const Ideal::DisplayResolution Ideal::D3D12RayTracingRenderer::m_resolutionOptions[] =
{
	{ 800u, 600u },
	//{ 1280u, 960u },
	{ 1200u, 900u },
	{ 1280u, 720u },
	{ 1920u, 1080u },
	{ 1920u, 1200u },
	{ 2560u, 1440u },
	{ 3440u, 1440u },
	{ 3840u, 2160u }
};

Ideal::D3D12RayTracingRenderer::D3D12RayTracingRenderer(HWND hwnd, uint32 Width, uint32 Height, bool EditorMode)
	: m_hwnd(hwnd),
	m_width(Width),
	m_height(Height),
	m_frameIndex(0),
	m_fenceEvent(NULL),
	m_fenceValue(0),
	m_isEditor(EditorMode)
{
	m_aspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
}

Ideal::D3D12RayTracingRenderer::~D3D12RayTracingRenderer()
{
	Fence();
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		WaitForFenceValue(m_lastFenceValues[i]);
		m_deferredDeleteManager->DeleteDeferredResources(i);
	}

	if (m_tearingSupport)
	{
		Check(m_swapChain->SetFullscreenState(FALSE, nullptr));
	}

	m_skyBoxTexture.reset();

	if (m_isEditor)
	{
		m_imguiSRVHandle.Free();
		m_editorTexture->Release();
		m_editorTexture->Free();
		m_editorTexture.reset();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	m_resourceManager->GetDefaultMaterial()->FreeInRayTracing();
	m_raytracingManager = nullptr;
	m_resourceManager = nullptr;
}

void Ideal::D3D12RayTracingRenderer::Init()
{
	uint32 dxgiFactoryFlags = 0;

#ifdef _DEBUG
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}
#endif

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ComPtr<ID3D12Debug1> debugController1;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			/*if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
			{
				debugController1->SetEnableGPUBasedValidation(true);
			}*/
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//---------------------Viewport Init-------------------------//
	m_viewport = std::make_shared<Ideal::D3D12Viewport>(m_width, m_height);
	m_viewport->Init();

	m_postViewport = std::make_shared<Ideal::D3D12Viewport>(m_width, m_height);
	m_postViewport->Init();

	//---------------------Create Device-------------------------//
	ComPtr<IDXGIFactory6> factory;
	Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf())), L"Failed To Create DXGIFactory1");

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1
	};
	DWORD featureLevelNum = _countof(featureLevels);
	DXGI_ADAPTER_DESC1 adapterDesc = {};
	ComPtr<IDXGIAdapter1> adapter = nullptr;
	for (uint32 featureLevelIndex = 0; featureLevelIndex < featureLevelNum; ++featureLevelIndex)
	{
		uint32 adapterIndex = 0;
		while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
		{
			adapter->GetDesc1(&adapterDesc);

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevels[featureLevelIndex], IID_PPV_ARGS(m_device.GetAddressOf()))))
			{
				DXGI_ADAPTER_DESC1 desc = {};
				adapter->GetDesc1(&desc);

				goto finishAdapter;
				break;
			}
			adapter = nullptr;
			adapterIndex++;
		}
	}
finishAdapter:

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 caps = {};
	Check(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &caps, sizeof(caps)), L"Device dose not support ray tracing!");
	if (caps.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		MyMessageBoxW(L"Device dose not support ray tracing!");
	}

	//D3D12_FEATURE_DATA_D3D12_OPTIONS17 option17 = {};
	//bool ManualWriteTrackingResourceSupported = false;
	//Check(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS17, &option17, sizeof(option17)), L"Device dose not support GPU Upload Heaps!");

	//---------------Command Queue-----------------//
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	Check(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf())));

	//---------------Create rendering resource---------------//
	CreateCommandlists();
	CreateSwapChains(factory);
	CreateRTV();
	CreateDSVHeap();
	CreateDSV(m_width, m_height);
	CreateFence();
	CreatePools();

	CreatePostScreenRootSignature();
	CreatePostScreenPipelineState();

	//---------------Compile Default Shader---------------//
	CompileDefaultShader();

	//------------------UI-------------------//
	CreateUIDescriptorHeap();
	CreateCanvas();

	//---------------Create Managers---------------//

	m_deferredDeleteManager = std::make_shared<Ideal::DeferredDeleteManager>();

	m_resourceManager = std::make_shared<Ideal::ResourceManager>();
	m_resourceManager->Init(m_device, m_deferredDeleteManager);
	m_resourceManager->SetAssetPath(m_assetPath);
	m_resourceManager->SetModelPath(m_modelPath);
	m_resourceManager->SetTexturePath(m_texturePath);
	m_resourceManager->CreateDefaultTextures();
	m_resourceManager->CreateDefaultMaterial();

	m_textManager = std::make_shared<Ideal::D2DTextManager>();
	m_textManager->Init(m_device, m_commandQueue);

	//------------------Particle System Manager-------------------//
	CreateParticleSystemManager();


	//------------------Debug Mesh Manager-------------------//
	if (m_isEditor)
	{
		CreateDebugMeshManager();
	}

	//---------------Editor---------------//
	if (m_isEditor)
	{
		//------ImGuiSRVHeap------//
		m_imguiSRVHeap = std::make_shared<Ideal::D3D12DynamicDescriptorHeap>();
		m_imguiSRVHeap->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, MAX_EDITOR_SRV_COUNT);

		//---------------------Editor RTV-------------------------//
		m_editorRTVHeap = std::make_shared<Ideal::D3D12DynamicDescriptorHeap>();
		m_editorRTVHeap->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 1);

		CreateEditorRTV(m_width, m_height);
		InitImGui();

		if (m_isEditor)
		{
			{
				auto srv = m_resourceManager->GetDefaultAlbedoTexture()->GetSRV();
				auto dest = m_imguiSRVHeap->Allocate();
				m_device->CopyDescriptorsSimple(1, dest.GetCpuHandle(), srv.GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				m_resourceManager->GetDefaultAlbedoTexture()->EmplaceSRVInEditor(dest);
			}
			{
				auto srv = m_resourceManager->GetDefaultNormalTexture()->GetSRV();
				auto dest = m_imguiSRVHeap->Allocate();
				m_device->CopyDescriptorsSimple(1, dest.GetCpuHandle(), srv.GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				m_resourceManager->GetDefaultNormalTexture()->EmplaceSRVInEditor(dest);
			}
			{
				auto srv = m_resourceManager->GetDefaultMaskTexture()->GetSRV();
				auto dest = m_imguiSRVHeap->Allocate();
				m_device->CopyDescriptorsSimple(1, dest.GetCpuHandle(), srv.GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				m_resourceManager->GetDefaultMaskTexture()->EmplaceSRVInEditor(dest);
			}
		}
	}
	m_sceneCB.CameraPos = Vector4(0.f);

	m_sceneCB.maxRadianceRayRecursionDepth = G_MAX_RAY_RECURSION_DEPTH;
	m_sceneCB.maxShadowRayRecursionDepth = G_MAX_RAY_RECURSION_DEPTH;

	m_sceneCB.nearZ = 0;
	m_sceneCB.farZ = 0;
	//m_sceneCB.nearZ = m_mainCamera->GetNearZ();
	//m_sceneCB.farZ = m_mainCamera->GetFarZ();

	// load image

	// create resource
	//CreateDeviceDependentResources();

	RaytracingManagerInit();
	m_raytracingManager->CreateMaterialInRayTracing(m_device, m_descriptorManager, m_resourceManager->GetDefaultMaterial());


	SetRendererAmbientIntensity(0.4f);
}

void Ideal::D3D12RayTracingRenderer::Tick()
{
	// 테스트 용으로 쓰는 곳
	__debugbreak();
	return;
}

void Ideal::D3D12RayTracingRenderer::Render()
{
#ifdef USE_UPLOAD_CONTAINER
	m_resourceManager->WaitForResourceUpload();
#endif

	m_mainCamera->UpdateMatrix2();
	//m_mainCamera->UpdateViewMatrix();

	m_sceneCB.CameraPos = m_mainCamera->GetPosition();
	m_sceneCB.ProjToWorld = m_mainCamera->GetViewProj().Invert().Transpose();
	m_sceneCB.View = m_mainCamera->GetView().Transpose();
	m_sceneCB.Proj = m_mainCamera->GetProj().Transpose();
	m_sceneCB.nearZ = m_mainCamera->GetNearZ();
	m_sceneCB.farZ = m_mainCamera->GetFarZ();

	m_sceneCB.resolutionX = m_width;
	m_sceneCB.resolutionY = m_height;

	m_sceneCB.resolutionX = m_postViewport->GetViewport().Width;
	m_sceneCB.resolutionY = m_postViewport->GetViewport().Height;

	m_sceneCB.FOV = m_mainCamera->GetFOV();


	m_globalCB.View = m_mainCamera->GetView().Transpose();
	m_globalCB.Proj = m_mainCamera->GetProj().Transpose();
	m_globalCB.ViewProj = m_mainCamera->GetViewProj().Transpose();
	m_globalCB.eyePos = m_mainCamera->GetPosition();

	UpdateLightListCBData();

	ResetCommandList();

#ifdef _DEBUG
	if (m_isEditor)
	{
		/*SetImGuiCameraRenderTarget();
		DrawImGuiMainCamera();*/
	}
#endif

	BeginRender();

	//---------------------Raytracing-------------------------//
	for (auto& mesh : m_staticMeshObject)
	{
		mesh->UpdateBLASInstance(m_raytracingManager);
	}
	for (auto& mesh : m_skinnedMeshObject)
	{
		mesh->UpdateBLASInstance(m_device,
			m_commandLists[m_currentContextIndex],
			m_resourceManager,
			m_descriptorManager,
			m_currentContextIndex,
			m_cbAllocator[m_currentContextIndex],
			m_raytracingManager
		);
	}

	RaytracingManagerUpdate();

	m_raytracingManager->DispatchRays(
		m_device,
		m_commandLists[m_currentContextIndex],
		m_descriptorManager,
		m_currentContextIndex,
		m_cbAllocator[m_currentContextIndex],
		m_sceneCB, &m_lightListCB, m_skyBoxTexture);

#ifdef BeforeRefactor
	CopyRaytracingOutputToBackBuffer();
#else
	TransitionRayTracingOutputToRTV();
#endif
	//---------Particle---------//
	DrawParticle();

	//----Debug Mesh Draw----//
	if (m_isEditor)
	{
		DrawDebugMeshes();
	}

	//-----------UI-----------//
	// Update Text Or Dynamic Texture 
	// Draw Text and Texture
	DrawCanvas();
#ifndef BeforeRefactor
	TransitionRayTracingOutputToSRV();
	// Final
	DrawPostScreen();
#endif
	//TransitionRayTracingOutputToUAV();
	//CopyRaytracingOutputToBackBuffer();

	if (m_isEditor)
	{
		ComPtr<ID3D12GraphicsCommandList4> commandlist = m_commandLists[m_currentContextIndex];
		ComPtr<ID3D12Resource> raytracingOutput = m_renderTargets[m_frameIndex]->GetResource();


		CD3DX12_RESOURCE_BARRIER preCopyBarriers[2];
		preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
			m_editorTexture->GetResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
			raytracingOutput.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		);
		commandlist->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
		commandlist->CopyResource(m_editorTexture->GetResource(), raytracingOutput.Get());

		CD3DX12_RESOURCE_BARRIER postCopyBarriers[2];
		postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
			m_editorTexture->GetResource(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
			raytracingOutput.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		commandlist->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
		DrawImGuiMainCamera();
	}

	//---------------------Editor-------------------------//
	if (m_isEditor)
	{
		ImGuiIO& io = ImGui::GetIO();
		ComPtr<ID3D12GraphicsCommandList> commandList = m_commandLists[m_currentContextIndex];
		//------IMGUI RENDER------//
		//ID3D12DescriptorHeap* descriptorHeap[] = { m_resourceManager->GetImguiSRVHeap().Get() };
		ID3D12DescriptorHeap* descriptorHeap[] = { m_imguiSRVHeap->GetDescriptorHeap().Get() };
		commandList->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)commandList.Get());
		}
	}

	//---------------------Present-------------------------//
	EndRender();
	Present();
	return;
}

void Ideal::D3D12RayTracingRenderer::Resize(UINT Width, UINT Height)
{
	if (!(Width * Height))
		return;
	if (m_width == Width && m_height == Height)
		return;

	// Wait For All Commands
	Fence();
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		WaitForFenceValue(m_lastFenceValues[i]);
	}
	DXGI_SWAP_CHAIN_DESC1 desc;
	HRESULT hr = m_swapChain->GetDesc1(&desc);
	for (uint32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; ++i)
	{
		m_renderTargets[i]->Release();
	}

	// ResizeBuffers
	Check(m_swapChain->ResizeBuffers(SWAP_CHAIN_FRAME_COUNT, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, m_swapChainFlags));

	BOOL isFullScreenState = false;
	Check(m_swapChain->GetFullscreenState(&isFullScreenState, nullptr));
	m_windowMode = !isFullScreenState;

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	for (uint32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; ++i)
	{
		auto handle = m_renderTargets[i]->GetRTV();
		ComPtr<ID3D12Resource> resource = m_renderTargets[i]->GetResource();
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
		m_device->CreateRenderTargetView(resource.Get(), nullptr, handle.GetCpuHandle());

		m_renderTargets[i]->Create(resource, m_deferredDeleteManager);
		m_renderTargets[i]->EmplaceRTV(handle);
	}

	// CreateDepthStencil
	//CreateDSV(Width, Height);

	m_width = Width;
	m_height = Height;

	// Viewport Reize
	m_viewport->ReSize(Width, Height);
	m_postViewport->ReSize(Width, Height);

	{
		auto r = m_resolutionOptions[m_displayResolutionIndex];
		m_postViewport->UpdatePostViewAndScissor(r.Width, r.Height);
	}
	//m_postViewport->UpdatePostViewAndScissor(Width, Height);

	if (m_mainCamera)
	{
		auto r = m_resolutionOptions[m_displayResolutionIndex];
		m_mainCamera->SetAspectRatio(float(r.Width) / r.Height);
	}

	if (m_isEditor)
	{
		//auto r = m_resolutionOptions[m_displayResolutionIndex];
		CreateEditorRTV(Width, Height);
	}

	// ray tracing / UI //
	//m_raytracingManager->Resize(m_device, Width, Height);
	m_UICanvas->SetCanvasSize(Width, Height);
	//
	SetDisplayResolutionOption(m_displayResolutionIndex);
}
void Ideal::D3D12RayTracingRenderer::ToggleFullScreenWindow()
{
	if (m_fullScreenMode)
	{
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(
			m_hwnd,
			HWND_NOTOPMOST,
			m_windowRect.left, m_windowRect.top, m_windowRect.right - m_windowRect.left, m_windowRect.bottom - m_windowRect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE
		);
		ShowWindow(m_hwnd, SW_NORMAL);
	}
	else
	{
		// 기존 윈도우의 Rect를 저장하여 전체화면 모드를 빠져나올 때 돌아올 수 있다.
		GetWindowRect(m_hwnd, &m_windowRect);

		// 윈도우를 경계없이 만들어서 클라이언트가 화면 전체를 채울 수 있게 한다.
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullScreenWindowRect;
		if (m_swapChain)
		{
			ComPtr<IDXGIOutput> output;
			Check(m_swapChain->GetContainingOutput(&output));
			DXGI_OUTPUT_DESC Desc;
			Check(output->GetDesc(&Desc));
			fullScreenWindowRect = Desc.DesktopCoordinates;
		}
		else
		{
			DEVMODE devMode = {};
			devMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

			fullScreenWindowRect =
			{
				devMode.dmPosition.x,
				devMode.dmPosition.y,
				devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
				devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
			};
		}

		SetWindowPos(
			m_hwnd,
			//HWND_TOPMOST,
			HWND_NOTOPMOST,
			fullScreenWindowRect.left,
			fullScreenWindowRect.top,
			fullScreenWindowRect.right,
			fullScreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(m_hwnd, SW_MAXIMIZE);
	}

	m_fullScreenMode = !m_fullScreenMode;
	SetDisplayResolutionOption(m_displayResolutionIndex);
}

bool Ideal::D3D12RayTracingRenderer::IsFullScreen()
{
	return m_fullScreenMode;
}

void Ideal::D3D12RayTracingRenderer::SetDisplayResolutionOption(const Resolution::EDisplayResolutionOption& Resolution)
{
	m_displayResolutionIndex = Resolution;
	// Wait For All Commands
	Fence();
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		WaitForFenceValue(m_lastFenceValues[i]);
	}

	uint32 resolutionWidth = m_resolutionOptions[Resolution].Width;
	uint32 resolutionHeight = m_resolutionOptions[Resolution].Height;

	m_viewport->ReSize(resolutionWidth, resolutionHeight);
	m_postViewport->UpdatePostViewAndScissor(resolutionWidth, resolutionHeight);

	if (m_mainCamera)
	{
		m_mainCamera->SetAspectRatio(float(resolutionWidth) / resolutionHeight);
	}

	// ray tracing / UI //
	//m_depthStencil.Reset();
	CreateDSV(resolutionWidth, resolutionHeight);
	m_raytracingManager->Resize(m_resourceManager, m_device, resolutionWidth, resolutionHeight);
	m_UICanvas->SetCanvasSize(resolutionWidth, resolutionHeight);
}

std::shared_ptr<Ideal::ICamera> Ideal::D3D12RayTracingRenderer::CreateCamera()
{
	std::shared_ptr<Ideal::IdealCamera> newCamera = std::make_shared<Ideal::IdealCamera>();
	newCamera->SetLens(0.25f * 3.141592f, m_aspectRatio, 1.f, 3000.f);
	return newCamera;
}

void Ideal::D3D12RayTracingRenderer::SetMainCamera(std::shared_ptr<ICamera> Camera)
{
	m_mainCamera = std::static_pointer_cast<Ideal::IdealCamera>(Camera);
}

std::shared_ptr<Ideal::IMeshObject> Ideal::D3D12RayTracingRenderer::CreateStaticMeshObject(const std::wstring& FileName)
{
	std::shared_ptr<Ideal::IdealStaticMeshObject> newStaticMesh = std::make_shared<Ideal::IdealStaticMeshObject>();
	m_resourceManager->CreateStaticMeshObject(newStaticMesh, FileName, false);
	newStaticMesh->SetName(FileName);
	//newStaticMesh->Init(m_device);

	// temp
	auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(newStaticMesh);

	RaytracingManagerAddObject(mesh);

	m_staticMeshObject.push_back(mesh);
	return newStaticMesh;
	//return nullptr;
}

std::shared_ptr<Ideal::ISkinnedMeshObject> Ideal::D3D12RayTracingRenderer::CreateSkinnedMeshObject(const std::wstring& FileName)
{
	std::shared_ptr<Ideal::IdealSkinnedMeshObject> newSkinnedMesh = std::make_shared<Ideal::IdealSkinnedMeshObject>();
	m_resourceManager->CreateSkinnedMeshObject(newSkinnedMesh, FileName);
	newSkinnedMesh->SetName(FileName);

	auto mesh = std::static_pointer_cast<Ideal::IdealSkinnedMeshObject>(newSkinnedMesh);
	RaytracingManagerAddObject(mesh);
	m_skinnedMeshObject.push_back(mesh);
	// 인터페이스로 따로 뽑아야 할 듯
	return newSkinnedMesh;
}

void Ideal::D3D12RayTracingRenderer::DeleteMeshObject(std::shared_ptr<Ideal::IMeshObject> MeshObject)
{
	if (MeshObject == nullptr)
		return;

	if (MeshObject->GetMeshType() == Ideal::Static)
	{
		auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(MeshObject);
		RaytracingManagerDeleteObject(mesh);
		m_resourceManager->DeleteStaticMeshObject(mesh);
		auto it = std::find(m_staticMeshObject.begin(), m_staticMeshObject.end(), mesh);
		{
			if (it != m_staticMeshObject.end())
			{
				*it = std::move(m_staticMeshObject.back());
				m_deferredDeleteManager->AddMeshObjectToDeferredDelete(MeshObject);
				m_staticMeshObject.pop_back();
			}
		}
	}
	else if (MeshObject->GetMeshType() == Ideal::Skinned)
	{
		auto mesh = std::static_pointer_cast<Ideal::IdealSkinnedMeshObject>(MeshObject);
		RaytracingManagerDeleteObject(mesh);
		m_resourceManager->DeleteSkinnedMeshObject(mesh);
		auto it = std::find(m_skinnedMeshObject.begin(), m_skinnedMeshObject.end(), mesh);
		{
			if (it != m_skinnedMeshObject.end())
			{
				*it = std::move(m_skinnedMeshObject.back());
				m_deferredDeleteManager->AddMeshObjectToDeferredDelete(MeshObject);
				m_skinnedMeshObject.pop_back();
			}
		}
	}
}

void Ideal::D3D12RayTracingRenderer::DeleteDebugMeshObject(std::shared_ptr<Ideal::IMeshObject> DebugMeshObject)
{
	// 아직 디버그 매쉬를 안만들고 static mesh에서 그냥 만드니 여기서 삭제
	auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(DebugMeshObject);
	//RaytracingManagerDeleteObject(mesh);
	//
	//auto it = std::find(m_staticMeshObject.begin(), m_staticMeshObject.end(), mesh);
	//{
	//	*it = std::move(m_staticMeshObject.back());
	//	m_deferredDeleteManager->AddMeshObjectToDeferredDelete(DebugMeshObject);
	//	m_staticMeshObject.pop_back();
	//}
	m_deferredDeleteManager->AddMeshObjectToDeferredDelete(DebugMeshObject);
	m_debugMeshManager->DeleteDebugMesh(mesh);
}

std::shared_ptr<Ideal::IMeshObject> Ideal::D3D12RayTracingRenderer::CreateDebugMeshObject(const std::wstring& FileName)
{
	std::shared_ptr<Ideal::IdealStaticMeshObject> newStaticMesh = std::make_shared<Ideal::IdealStaticMeshObject>();
	m_resourceManager->CreateStaticMeshObject(newStaticMesh, FileName);
	newStaticMesh->SetName(FileName);
	auto mesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(newStaticMesh);
	mesh->SetDebugMeshColor(Color(0, 1, 0, 1));

	if (m_isEditor)
	{
		m_debugMeshManager->AddDebugMesh(mesh);
	}
	return mesh;
}

std::shared_ptr<Ideal::IAnimation> Ideal::D3D12RayTracingRenderer::CreateAnimation(const std::wstring& FileName, const Matrix& offset /*= Matrix::Identity*/)
{
	std::shared_ptr<Ideal::IdealAnimation> newAnimation = std::make_shared<Ideal::IdealAnimation>();
	m_resourceManager->CreateAnimation(newAnimation, FileName, offset);

	return newAnimation;
}

std::shared_ptr<Ideal::IDirectionalLight> Ideal::D3D12RayTracingRenderer::CreateDirectionalLight()
{
	auto newLight = std::make_shared<Ideal::IdealDirectionalLight>();
	m_directionalLights.push_back(newLight);
	return newLight;
}

std::shared_ptr<Ideal::ISpotLight> Ideal::D3D12RayTracingRenderer::CreateSpotLight()
{
	auto newLight = std::make_shared<Ideal::IdealSpotLight>();
	m_spotLights.push_back(newLight);
	return newLight;
}

std::shared_ptr<Ideal::IPointLight> Ideal::D3D12RayTracingRenderer::CreatePointLight()
{
	auto newLight = std::make_shared<Ideal::IdealPointLight>();
	m_pointLights.push_back(newLight);
	return newLight;
}

void Ideal::D3D12RayTracingRenderer::DeleteLight(std::shared_ptr<Ideal::ILight> Light)
{
	auto lightType = Light->GetLightType();
	switch (lightType)
	{
		case ELightType::None:
			break;
		case ELightType::Directional:
		{
			auto castLight = std::static_pointer_cast<Ideal::IdealDirectionalLight>(Light);
			auto it = std::find(m_directionalLights.begin(), m_directionalLights.end(), castLight);

			//auto it = std::find_if(m_directionalLights.begin(), m_directionalLights.end(),
			//	[&castLight](const std::shared_ptr<Ideal::IdealDirectionalLight>& light) {
			//		return light == castLight; // Compare the shared pointers
			//	});

			{
				if (it != m_directionalLights.end())
				{
					*it = std::move(m_directionalLights.back());
					m_directionalLights.pop_back();
				}
			}
		}
		break;
		case ELightType::Spot:
		{
			auto castLight = std::static_pointer_cast<Ideal::IdealSpotLight>(Light);
			auto it = std::find(m_spotLights.begin(), m_spotLights.end(), castLight);
			//auto it = std::find_if(m_spotLights.begin(), m_spotLights.end(),
			//	[&castLight](const std::shared_ptr<Ideal::IdealSpotLight>& light) {
			//		return light == castLight; // Compare the shared pointers
			//	});
			{
				if (it != m_spotLights.end())
				{
					*it = std::move(m_spotLights.back());
					m_spotLights.pop_back();
				}
			}
		}
		break;
		case ELightType::Point:
		{
			auto castLight = std::static_pointer_cast<Ideal::IdealPointLight>(Light);
			auto it = std::find(m_pointLights.begin(), m_pointLights.end(), castLight);
			//auto it = std::find_if(m_pointLights.begin(), m_pointLights.end(),
			//	[&castLight](const std::shared_ptr<Ideal::IdealPointLight>& light) {
			//		return light == castLight; // Compare the shared pointers
			//	});
			{
				if (it != m_pointLights.end())
				{
					*it = std::move(m_pointLights.back());
					m_pointLights.pop_back();
				}
			}
		}
		break;
		default:
			break;
	}
}

void Ideal::D3D12RayTracingRenderer::SetAssetPath(const std::wstring& AssetPath)
{
	m_assetPath = AssetPath;
}

void Ideal::D3D12RayTracingRenderer::SetModelPath(const std::wstring& ModelPath)
{
	m_modelPath = ModelPath;
}

void Ideal::D3D12RayTracingRenderer::SetTexturePath(const std::wstring& TexturePath)
{
	m_texturePath = TexturePath;
}

void Ideal::D3D12RayTracingRenderer::ConvertAssetToMyFormat(std::wstring FileName, bool isSkinnedData /*= false*/, bool NeedVertexInfo /*= false*/, bool NeedConvertCenter /*= false*/)
{
	std::shared_ptr<AssimpConverter> assimpConverter = std::make_shared<AssimpConverter>();
	assimpConverter->SetAssetPath(m_assetPath);
	assimpConverter->SetModelPath(m_modelPath);
	assimpConverter->SetTexturePath(m_texturePath);

	assimpConverter->ReadAssetFile(FileName, isSkinnedData, NeedVertexInfo);

	// Temp : ".fbx" 삭제
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();

	assimpConverter->ExportModelData(FileName, isSkinnedData, NeedConvertCenter);
	if (NeedVertexInfo)
	{
		assimpConverter->ExportVertexPositionData(FileName);
	}
	assimpConverter->ExportMaterialData(FileName);
}

void Ideal::D3D12RayTracingRenderer::ConvertAnimationAssetToMyFormat(std::wstring FileName)
{
	std::shared_ptr<AssimpConverter> assimpConverter = std::make_shared<AssimpConverter>();
	assimpConverter->SetAssetPath(m_assetPath);
	assimpConverter->SetModelPath(m_modelPath);
	assimpConverter->SetTexturePath(m_texturePath);

	assimpConverter->ReadAssetFile(FileName, false, false);

	// Temp : ".fbx" 삭제
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();

	assimpConverter->ExportAnimationData(FileName);
}

void Ideal::D3D12RayTracingRenderer::ConvertParticleMeshAssetToMyFormat(std::wstring FileName, bool SetScale /*= false*/, Vector3 Scale /*= Vector3(1.f)*/)
{
	std::shared_ptr<AssimpConverter> assimpConverter = std::make_shared<AssimpConverter>();
	assimpConverter->SetAssetPath(m_assetPath);
	assimpConverter->SetModelPath(m_modelPath);
	assimpConverter->SetTexturePath(m_texturePath);

	assimpConverter->ReadAssetFile(FileName, false, false);

	// Temp : ".fbx" 삭제
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();
	FileName.pop_back();

	assimpConverter->ExportParticleData(FileName, SetScale, Scale);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Ideal::D3D12RayTracingRenderer::SetImGuiWin32WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (::ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
}

void Ideal::D3D12RayTracingRenderer::ClearImGui()
{
	if (m_isEditor)
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();
	}
}

DirectX::SimpleMath::Vector2 Ideal::D3D12RayTracingRenderer::GetTopLeftEditorPos()
{
	if (m_isEditor)
	{
		//m_mainCameraEditorTopLeft;
		// 비율 계산
		auto& rect = m_postViewport->GetScissorRect();
		m_mainCameraEditorWindowSize;
		// offset * editor size / main window size = new Offset // return new Offset + editor pos
		float y = rect.top * m_mainCameraEditorWindowSize.y / m_height;
		float x = rect.left * m_mainCameraEditorWindowSize.x / m_width;

		float ny = m_mainCameraEditorTopLeft.y;
		float nx = m_mainCameraEditorTopLeft.x;
		ny += y;
		nx += x;
		return Vector2(nx, ny);
	}
	else
	{
		if (m_fullScreenMode)
		{
			//uint32 width = GetSystemMetrics(SM_CXSCREEN);
			//uint32 height = GetSystemMetrics(SM_CYSCREEN);
			// 풀스크린일때 또 다르다?

			//RECT windowRect;
			//GetWindowRect(m_hwnd, &windowRect);
			//int32 x = windowRect.left;
			//int32 y = windowRect.top;
			//int32 width = windowRect.right - windowRect.left;
			//int32 height = windowRect.bottom - windowRect.top;

			float nx = m_postViewport->GetScissorRect().left;
			float ny = m_postViewport->GetScissorRect().top;

			return Vector2(nx, ny);

		}
		else
		{
			RECT windowRect;
			GetWindowRect(m_hwnd, &windowRect);
			int32 x = windowRect.left;
			int32 y = windowRect.top;
			int32 width = windowRect.right - windowRect.left;
			int32 height = windowRect.bottom - windowRect.top;

			int32 borderThickness = GetSystemMetrics(SM_CXSIZEFRAME);
			int32 titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
			float nx = x + m_postViewport->GetScissorRect().left + borderThickness + borderThickness;
			float ny = y + m_postViewport->GetScissorRect().top + borderThickness + borderThickness + titleBarHeight;

			return Vector2(nx, ny);
		}
	}
}

DirectX::SimpleMath::Vector2 Ideal::D3D12RayTracingRenderer::GetRightBottomEditorPos()
{
	if (m_isEditor)
	{
		//m_mainCameraEditorTopLeft;
		// 비율 계산
		auto& rect = m_postViewport->GetScissorRect();
		m_mainCameraEditorWindowSize;
		// offset * editor size / main window size = new Offset // return new Offset + editor pos
		float y = rect.top * m_mainCameraEditorWindowSize.y / m_height;
		float x = rect.left * m_mainCameraEditorWindowSize.x / m_width;

		float ny = m_mainCameraEditorBottomRight.y;
		float nx = m_mainCameraEditorBottomRight.x;
		ny -= y;
		nx -= x;
		return Vector2(nx, ny);
	}
	else
	{
		if (m_fullScreenMode)
		{
			uint32 width = GetSystemMetrics(SM_CXSCREEN);
			uint32 height = GetSystemMetrics(SM_CYSCREEN);
			// 풀스크린일때 또 다르다?

			float nx = m_postViewport->GetScissorRect().right;
			float ny = m_postViewport->GetScissorRect().bottom;

			return Vector2(nx, ny);
		}
		else
		{
			RECT windowRect;
			GetWindowRect(m_hwnd, &windowRect);
			int32 x = windowRect.left;
			int32 y = windowRect.top;
			int32 width = windowRect.right - windowRect.left;
			int32 height = windowRect.bottom - windowRect.top;

			int32 borderThickness = GetSystemMetrics(SM_CXSIZEFRAME);
			int32 titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
			float nx = x + m_postViewport->GetScissorRect().right + borderThickness + borderThickness;
			float ny = y + m_postViewport->GetScissorRect().bottom + borderThickness + borderThickness + titleBarHeight;

			return Vector2(nx, ny);
		}
	}
}

void Ideal::D3D12RayTracingRenderer::SetSkyBox(std::shared_ptr<Ideal::ITexture> SkyBoxTexture)
{
	if (m_skyBoxTexture)
	{
		m_deferredDeleteManager->AddD3D12ResourceToDelete(m_skyBoxTexture->GetResource());
	}
	m_skyBoxTexture = std::static_pointer_cast<Ideal::D3D12Texture>(SkyBoxTexture);
}

std::shared_ptr<Ideal::ITexture> Ideal::D3D12RayTracingRenderer::CreateSkyBox(const std::wstring& FileName)
{
	std::shared_ptr <Ideal::D3D12Texture> skyBox = std::make_shared<Ideal::D3D12Texture>();
	m_resourceManager->CreateTextureDDS(skyBox, FileName);
	return skyBox;
}

std::shared_ptr<Ideal::ITexture> Ideal::D3D12RayTracingRenderer::CreateTexture(const std::wstring& FileName, bool IsGenerateMips /*= false*/, bool IsNormalMap /*= false*/, bool IngoreSRGB /*= false*/)
{
	std::shared_ptr<Ideal::D3D12Texture> texture;
	uint32 generateMips = 1;
	if (IsGenerateMips)
	{
		generateMips = 0;
	}
	m_resourceManager->CreateTexture(texture, FileName, IngoreSRGB, generateMips,IsNormalMap);

	//if (IsGenerateMips)
	//{
	//	m_resourceManager->GenerateMips(
	//		m_device,
	//		m_commandLists[m_currentContextIndex],
	//		m_descriptorHeaps[m_currentContextIndex],
	//		m_cbAllocator[m_currentContextIndex],
	//		texture,
	//		4	// TEMP
	//	);
	//}


	if (m_isEditor)
	{
		auto srv = texture->GetSRV();
		auto dest = m_imguiSRVHeap->Allocate();
		m_device->CopyDescriptorsSimple(1, dest.GetCpuHandle(), srv.GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		texture->EmplaceSRVInEditor(dest);
	}
	return texture;
}

std::shared_ptr<Ideal::IMaterial> Ideal::D3D12RayTracingRenderer::CreateMaterial()
{
	std::shared_ptr<Ideal::IMaterial> ret = m_resourceManager->CreateMaterial();
	m_raytracingManager->CreateMaterialInRayTracing(m_device, m_descriptorManager, std::static_pointer_cast<Ideal::IdealMaterial>(ret));
	return ret;
}

void Ideal::D3D12RayTracingRenderer::DeleteTexture(std::shared_ptr<Ideal::ITexture> Texture)
{
	if (!Texture) return;
	m_deferredDeleteManager->AddTextureToDeferredDelete(std::static_pointer_cast<Ideal::D3D12Texture>(Texture));
	m_resourceManager->DeleteTexture(std::static_pointer_cast<Ideal::D3D12Texture>(Texture));
}

void Ideal::D3D12RayTracingRenderer::DeleteMaterial(std::shared_ptr<Ideal::IMaterial> Material)
{
	if (!Material) return;
	m_deferredDeleteManager->AddMaterialToDefferedDelete(std::static_pointer_cast<Ideal::IdealMaterial>(Material));
	m_raytracingManager->DeleteMaterialInRayTracing(Material);
}

std::shared_ptr<Ideal::ISprite> Ideal::D3D12RayTracingRenderer::CreateSprite()
{
	// TODO : Canvas에 UI를 추가해야 할 것이다.
	std::shared_ptr<Ideal::IdealSprite> ret = std::make_shared<Ideal::IdealSprite>();
	//ret->SetScreenSize(Vector2(m_width, m_height));
	auto r = m_resolutionOptions[m_displayResolutionIndex];
	ret->SetScreenSize(Vector2(r.Width, r.Height));
	ret->SetMesh(m_resourceManager->GetDefaultQuadMesh());
	ret->SetTexture(m_resourceManager->GetDefaultAlbedoTexture());
	m_UICanvas->AddSprite(ret);
	return ret;
}

void Ideal::D3D12RayTracingRenderer::DeleteSprite(std::shared_ptr<Ideal::ISprite>& Sprite)
{
	auto s = std::static_pointer_cast<Ideal::IdealSprite>(Sprite);
	m_UICanvas->DeleteSprite(s);
	Sprite.reset();
}

std::shared_ptr<Ideal::IText> Ideal::D3D12RayTracingRenderer::CreateText(uint32 Width, uint32 Height, float FontSize, std::wstring Font /*= L"Tahoma"*/)
{
	auto fontHandle = m_textManager->CreateTextObject(Font.c_str(), FontSize);
	auto text = std::make_shared<Ideal::IdealText>(m_textManager, fontHandle);
	std::shared_ptr<Ideal::D3D12Texture> dynamicTexture;
	m_resourceManager->CreateDynamicTexture(dynamicTexture, Width, Height);
	{
		//Sprite
		std::shared_ptr<Ideal::IdealSprite> sprite = std::make_shared<Ideal::IdealSprite>();
		//sprite->SetScreenSize(Vector2(m_width, m_height));
		auto r = m_resolutionOptions[m_displayResolutionIndex];
		sprite->SetScreenSize(Vector2(r.Width, r.Height));
		sprite->SetMesh(m_resourceManager->GetDefaultQuadMesh());
		sprite->SetTexture(m_resourceManager->GetDefaultAlbedoTexture());
		text->SetSprite(sprite);
	}
	text->SetTexture(dynamicTexture);
	text->ChangeText(L"Text : 텍스트");
	text->UpdateDynamicTextureWithImage(m_device);
	m_UICanvas->AddText(text);
	return text;
}

void Ideal::D3D12RayTracingRenderer::DeleteText(std::shared_ptr<Ideal::IText>& Text)
{
	if (Text)
	{
		m_UICanvas->DeleteText(std::static_pointer_cast<Ideal::IdealText>(Text));
		// Text는 그래픽엔진에서 만드는 텍스쳐라 삭제.
		//m_deferredDeleteManager->AddD3D12ResourceToDelete((std::static_pointer_cast<Ideal::IdealText>(Text))->GetTexture()->GetResource());
		m_deferredDeleteManager->AddTextureToDeferredDelete((std::static_pointer_cast<Ideal::IdealText>(Text))->GetTexture());
	}
}

void Ideal::D3D12RayTracingRenderer::CompileShader(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName, const std::wstring& ShaderVersion, const std::wstring& EntryPoint /*= L"Main"*/, const std::wstring& IncludeFilePath /*= L""*/, bool HasEntry /*= true*/)
{
	ComPtr<IDxcBlob> blob;
	// TODO : 쉐이더를 컴파일 해야 한다.
	std::shared_ptr<Ideal::ShaderManager> shaderManager = std::make_shared<Ideal::ShaderManager>();
	shaderManager->Init();
	shaderManager->AddIncludeDirectories(IncludeFilePath);
	shaderManager->CompileShaderAndSave(
		FilePath.c_str(),
		SavePath,
		SaveName.c_str(),
		ShaderVersion,
		blob,
		EntryPoint, //entry
		HasEntry
	);
}

std::shared_ptr<Ideal::IShader> Ideal::D3D12RayTracingRenderer::CreateAndLoadParticleShader(const std::wstring& Name)
{
	std::shared_ptr<Ideal::ShaderManager> shaderManager = std::make_shared<Ideal::ShaderManager>();
	std::shared_ptr<Ideal::D3D12Shader> shader = std::make_shared<Ideal::D3D12Shader>();
	shaderManager->Init();

	// TODO : 경로와 Name을 더해주어야 한다.
	std::wstring FilePath = L"../Shaders/Particle/" + Name + L".shader";

	shaderManager->LoadShaderFile(FilePath, shader);
	return std::static_pointer_cast<Ideal::IShader>(shader);
}

std::shared_ptr<Ideal::D3D12Shader> Ideal::D3D12RayTracingRenderer::CreateAndLoadShader(const std::wstring& FilePath)
{
	std::shared_ptr<Ideal::ShaderManager> shaderManager = std::make_shared<Ideal::ShaderManager>();
	std::shared_ptr<Ideal::D3D12Shader> shader = std::make_shared<Ideal::D3D12Shader>();
	shaderManager->Init();
	shaderManager->LoadShaderFile(FilePath, shader);
	return shader;
}

std::shared_ptr<Ideal::IParticleSystem> Ideal::D3D12RayTracingRenderer::CreateParticleSystem(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial)
{
	std::shared_ptr<Ideal::ParticleSystem> NewParticleSystem = std::make_shared<Ideal::ParticleSystem>();
	std::shared_ptr<Ideal::ParticleMaterial> GetParticleMaterial = std::static_pointer_cast<Ideal::ParticleMaterial>(ParticleMaterial);
	NewParticleSystem->Init(m_device, m_particleSystemManager->GetRootSignature(), GetParticleMaterial);
	NewParticleSystem->SetMeshVS(m_particleSystemManager->GetMeshVS());
	NewParticleSystem->SetBillboardVS(m_particleSystemManager->GetBillboardVS());
	NewParticleSystem->SetBillboardGS(m_particleSystemManager->GetBillboardGS());
	NewParticleSystem->SetParticleVertexBuffer(m_particleSystemManager->GetParticleVertexBuffer());
	NewParticleSystem->SetBillboardCalculateComputePipelineState(m_particleSystemManager->GetParticleComputePipelineState());

	NewParticleSystem->SetResourceManager(m_resourceManager);
	NewParticleSystem->SetDeferredDeleteManager(m_deferredDeleteManager);

	if (GetParticleMaterial->GetTransparency())
	{
		m_particleSystemManager->AddParticleSystem(NewParticleSystem);
	}
	else
	{
		m_particleSystemManager->AddParticleSystemNoTransparency(NewParticleSystem);
	}
	return std::static_pointer_cast<Ideal::IParticleSystem>(NewParticleSystem);
}

void Ideal::D3D12RayTracingRenderer::DeleteParticleSystem(std::shared_ptr<Ideal::IParticleSystem>& ParticleSystem)
{
	m_particleSystemManager->DeleteParticleSystem(std::static_pointer_cast<Ideal::ParticleSystem>(ParticleSystem));
}

std::shared_ptr<Ideal::IParticleMaterial> Ideal::D3D12RayTracingRenderer::CreateParticleMaterial()
{
	std::shared_ptr<Ideal::ParticleMaterial> NewParticleMaterial = std::make_shared<Ideal::ParticleMaterial>();
	return std::static_pointer_cast<Ideal::IParticleMaterial>(NewParticleMaterial);
}

void Ideal::D3D12RayTracingRenderer::DeleteParticleMaterial(std::shared_ptr<Ideal::IParticleMaterial>& ParticleMaterial)
{

}

std::shared_ptr<Ideal::IMesh> Ideal::D3D12RayTracingRenderer::CreateParticleMesh(const std::wstring& FileName)
{
	auto mesh = m_resourceManager->CreateParticleMesh(FileName);
	return mesh;
}

void Ideal::D3D12RayTracingRenderer::SetRendererAmbientIntensity(float Value)
{
	m_sceneCB.AmbientIntensity = Value;
}

void Ideal::D3D12RayTracingRenderer::CreateSwapChains(ComPtr<IDXGIFactory6> Factory)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = SWAP_CHAIN_FRAME_COUNT;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;


	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
	//fullScreenDesc.RefreshRate.Numerator = 60;
	//fullScreenDesc.RefreshRate.Denominator = 1;
	//fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//fullScreenDesc.Windowed = true;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Stereo = FALSE;
	//swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	swapChainDesc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	m_swapChainFlags = swapChainDesc.Flags;

	ComPtr<IDXGISwapChain1> swapChain;
	Check(Factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		swapChain.GetAddressOf()
	));

	if (m_tearingSupport)
	{
		Check(Factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER), L"Failed to make window association");
	}
	//Check(Factory->MakeWindowAssociation(m_hwnd, NULL), L"Failed to make window association");
	Check(swapChain.As(&m_swapChain), L"Failed to change swapchain version");
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

}

void Ideal::D3D12RayTracingRenderer::CreateRTV()
{
	m_rtvHeap = std::make_shared<Ideal::D3D12DescriptorHeap>();
	m_rtvHeap->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, SWAP_CHAIN_FRAME_COUNT);
	//m_rtvHeap->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, SWAP_CHAIN_FRAME_COUNT);

	float c[4] = { 0.f,0.f,0.f,1.f };
	D3D12_CLEAR_VALUE clearValue = {};
	//clearValue.Format = swapChainDesc.Format;
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	clearValue.Color[0] = c[0];
	clearValue.Color[1] = c[1];
	clearValue.Color[2] = c[2];
	clearValue.Color[3] = c[3];

	{
		// Create RenderTarget Texture
		for (uint32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; ++i)
		{
			m_renderTargets[i] = std::make_shared<Ideal::D3D12Texture>();
		}

		for (uint32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; ++i)
		{
			auto handle = m_rtvHeap->Allocate();
			ComPtr<ID3D12Resource> resource;
			Check(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&resource)));
			m_device->CreateRenderTargetView(resource.Get(), nullptr, handle.GetCpuHandle());
			m_renderTargets[i]->Create(resource, m_deferredDeleteManager);
			m_renderTargets[i]->EmplaceRTV(handle);
		}
	}
}

void Ideal::D3D12RayTracingRenderer::CreateDSVHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Check(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

}

void Ideal::D3D12RayTracingRenderer::CreateDSV(uint32 Width, uint32 Height)
{
	if (m_depthStencil)
	{
		m_depthStencil->Release();
	}
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		Width,
		Height,
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	Check(m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(m_depthStencil.GetAddressOf())
	));

	// create dsv
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

}

void Ideal::D3D12RayTracingRenderer::CreateFence()
{
	Check(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())), L"Failed to create fence!");
	m_fenceValue = 0;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void Ideal::D3D12RayTracingRenderer::CreateCommandlists()
{
	// CreateCommand List
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		Check(
			m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocators[i].GetAddressOf())),
			L"Failed to create command allocator!"
		);

		Check(
			m_device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_commandAllocators[i].Get(),
				nullptr,
				IID_PPV_ARGS(m_commandLists[i].GetAddressOf())),
			L"Failed to create command list"
		);
		Check(m_commandLists[i]->Close(), L"Failed to close commandlist");
	}

}

void Ideal::D3D12RayTracingRenderer::CreatePools()
{
	// descriptor heap, constant buffer pool Allocator
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		// descriptor heap
		m_descriptorHeaps[i] = std::make_shared<Ideal::D3D12DescriptorHeap>();
		m_descriptorHeaps[i]->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, MAX_DESCRIPTOR_COUNT);

		// Dynamic CB Pool Allocator
		m_cbAllocator[i] = std::make_shared<Ideal::D3D12DynamicConstantBufferAllocator>();
		m_cbAllocator[i]->Init(MAX_DRAW_COUNT_PER_FRAME);

		// Upload Pool
		m_BLASInstancePool[i] = std::make_shared<Ideal::D3D12UploadBufferPool>();
		//m_BLASInstancePool[i]->Init(m_device.Get(), sizeof(Ideal::DXRInstanceDesc), MAX_DRAW_COUNT_PER_FRAME);
		m_BLASInstancePool[i]->Init(m_device.Get(), sizeof(Ideal::DXRInstanceDesc), MAX_DRAW_COUNT_PER_FRAME);
	}

	m_descriptorManager = std::make_shared<Ideal::D3D12DescriptorManager>();
	m_descriptorManager->Create(m_device, MAX_PENDING_FRAME_COUNT, MAX_DRAW_COUNT_PER_FRAME, 1, MAX_DRAW_COUNT_PER_FRAME * 4);
}

void Ideal::D3D12RayTracingRenderer::CreateDefaultCamera()
{
	m_mainCamera = std::make_shared<Ideal::IdealCamera>();
	std::shared_ptr<Ideal::IdealCamera> camera = std::static_pointer_cast<Ideal::IdealCamera>(m_mainCamera);
	camera->SetLens(0.25f * 3.141592f, m_aspectRatio, 1.f, 3000.f);
}

void Ideal::D3D12RayTracingRenderer::UpdatePostViewAndScissor(uint32 Width, uint32 Height)
{
	//float viewWidthRatio = static_cast<float>(Width) / m_width;
	//float viewHeightRatio = static_cast<float>(Height) / m_height;
	//
	//float x = 1.0f;
	//float y = 1.0f;
	//
	//if (viewWidthRatio < viewHeightRatio)
	//{
	//	// The scaled image's height will fit to the viewport's height and 
	//	// its width will be smaller than the viewport's width.
	//	x = viewWidthRatio / viewHeightRatio;
	//}
	//else
	//{
	//	// The scaled image's width will fit to the viewport's width and 
	//	// its height may be smaller than the viewport's height.
	//	y = viewHeightRatio / viewWidthRatio;
	//}
	//
	//m_viewport->ReSize()
	//
	//
	//m_postViewport.TopLeftX = m_width * (1.0f - x) / 2.0f;
	//m_postViewport.TopLeftY = m_height * (1.0f - y) / 2.0f;
	//m_postViewport.Width = x * m_width;
	//m_postViewport.Height = y * m_height;
	//
	//m_postScissorRect.left = static_cast<LONG>(m_postViewport.TopLeftX);
	//m_postScissorRect.right = static_cast<LONG>(m_postViewport.TopLeftX + m_postViewport.Width);
	//m_postScissorRect.top = static_cast<LONG>(m_postViewport.TopLeftY);
	//m_postScissorRect.bottom = static_cast<LONG>(m_postViewport.TopLeftY + m_postViewport.Height);
}

void Ideal::D3D12RayTracingRenderer::ResetCommandList()
{
	ComPtr<ID3D12CommandAllocator> commandAllocator = m_commandAllocators[m_currentContextIndex];
	ComPtr<ID3D12GraphicsCommandList> commandList = m_commandLists[m_currentContextIndex];

	Check(commandAllocator->Reset(), L"Failed to reset commandAllocator!");
	Check(commandList->Reset(commandAllocator.Get(), nullptr), L"Failed to reset commandList");
}

void Ideal::D3D12RayTracingRenderer::BeginRender()
{
	ComPtr<ID3D12CommandAllocator> commandAllocator = m_commandAllocators[m_currentContextIndex];
	ComPtr<ID3D12GraphicsCommandList> commandList = m_commandLists[m_currentContextIndex];

	CD3DX12_RESOURCE_BARRIER backBufferBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex]->GetResource(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList->ResourceBarrier(1, &backBufferBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	auto rtvHandle = m_renderTargets[m_frameIndex]->GetRTV();


	commandList->ClearRenderTargetView(rtvHandle.GetCpuHandle(), DirectX::Colors::Black, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &m_viewport->GetViewport());
	commandList->RSSetScissorRects(1, &m_viewport->GetScissorRect());

	//commandList->OMSetRenderTargets(1, &rtvHandle.GetCpuHandle(), FALSE, &dsvHandle);
	commandList->OMSetRenderTargets(1, &rtvHandle.GetCpuHandle(), FALSE, nullptr);
}

void Ideal::D3D12RayTracingRenderer::EndRender()
{
	ComPtr<ID3D12GraphicsCommandList> commandList = m_commandLists[m_currentContextIndex];

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex]->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	commandList->ResourceBarrier(1, &barrier);
	Check(commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void Ideal::D3D12RayTracingRenderer::Present()
{
	Fence();

	HRESULT hr;

	uint32 SyncInterval = 0;
	uint32 PresentFlags = 0;
	PresentFlags = DXGI_PRESENT_ALLOW_TEARING;
	PresentFlags = (m_tearingSupport && m_windowMode) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	hr = m_swapChain->Present(0, PresentFlags);
	//hr = m_swapChain->Present(1, 0);
	//hr = m_swapChain->Present(0, 0);
	Check(hr);

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	uint32 nextContextIndex = (m_currentContextIndex + 1) % MAX_PENDING_FRAME_COUNT;
	WaitForFenceValue(m_lastFenceValues[nextContextIndex]);

	m_descriptorHeaps[nextContextIndex]->Reset();
	m_cbAllocator[nextContextIndex]->Reset();
	//m_BLASInstancePool[nextContextIndex]->Reset();
	m_descriptorManager->ResetPool(nextContextIndex);
	// ui
	m_mainDescriptorHeaps[nextContextIndex]->Reset();

	// deferred resource Delete And Set Next Context Index
	m_deferredDeleteManager->DeleteDeferredResources(m_currentContextIndex);

	m_currentContextIndex = nextContextIndex;

}

uint64 Ideal::D3D12RayTracingRenderer::Fence()
{
	m_fenceValue++;
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	m_lastFenceValues[m_currentContextIndex] = m_fenceValue;

	return m_fenceValue;
}

void Ideal::D3D12RayTracingRenderer::WaitForFenceValue(uint64 ExpectedFenceValue)
{
	uint64 completedValue = m_fence->GetCompletedValue();
	if (m_fence->GetCompletedValue() < ExpectedFenceValue)
	{
		m_fence->SetEventOnCompletion(ExpectedFenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	uint64 completedValue1 = m_fence->GetCompletedValue();
}

void Ideal::D3D12RayTracingRenderer::CreatePostScreenRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::PostScreenRootSignature::Slot::Count];
	CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::PostScreenRootSignature::Slot::Count];

	ranges[Ideal::PostScreenRootSignature::Slot::SRV_Scene].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootParameters[Ideal::PostScreenRootSignature::Slot::SRV_Scene].InitAsDescriptorTable(1, &ranges[Ideal::PostScreenRootSignature::Slot::SRV_Scene], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_SIGNATURE_FLAGS rootSigantureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Create a sampler.
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSigantureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (error)
	{
		MyMessageBox((char*)error->GetBufferPointer());
	}
	Check(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_postScreenRootSignature)));
}

void Ideal::D3D12RayTracingRenderer::CreatePostScreenPipelineState()
{
	// TEMP : shader compile
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	ComPtr<ID3DBlob> error;
	ComPtr<ID3DBlob> postVertexShader;
	ComPtr<ID3DBlob> postPixelShader;
	Check(D3DCompileFromFile(L"../Shaders/Screen/Screen.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &postVertexShader, &error));
	if (error)
	{
		MyMessageBox((char*)error->GetBufferPointer());
	}
	Check(D3DCompileFromFile(L"../Shaders/Screen/Screen.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &postPixelShader, &error));
	if (error)
	{
		MyMessageBox((char*)error->GetBufferPointer());
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { SimpleVertex::InputElements, SimpleVertex::InputElementCount };
	psoDesc.pRootSignature = m_postScreenRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(postVertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(postPixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	Check(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_postScreenPipelineState)));
}

void Ideal::D3D12RayTracingRenderer::DrawPostScreen()
{
	std::shared_ptr<Ideal::D3D12Texture> renderTarget = m_renderTargets[m_frameIndex];
	m_commandLists[m_currentContextIndex]->RSSetViewports(1, &m_postViewport->GetViewport());
	m_commandLists[m_currentContextIndex]->RSSetScissorRects(1, &m_postViewport->GetScissorRect());
	m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &renderTarget->GetRTV().GetCpuHandle(), FALSE, nullptr);

	m_commandLists[m_currentContextIndex]->SetPipelineState(m_postScreenPipelineState.Get());
	m_commandLists[m_currentContextIndex]->SetGraphicsRootSignature(m_postScreenRootSignature.Get());
	m_commandLists[m_currentContextIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto defaultQuadMesh = m_resourceManager->GetDefaultQuadMesh2();

	m_commandLists[m_currentContextIndex]->IASetVertexBuffers(0, 1, &defaultQuadMesh->GetVertexBufferView());
	m_commandLists[m_currentContextIndex]->IASetIndexBuffer(&defaultQuadMesh->GetIndexBufferView());
	auto handle = m_mainDescriptorHeaps[m_currentContextIndex]->Allocate();
	m_device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), m_raytracingManager->GetRaytracingOutputSRVHandle().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_commandLists[m_currentContextIndex]->SetGraphicsRootDescriptorTable(Ideal::PostScreenRootSignature::Slot::SRV_Scene, handle.GetGpuHandle());
	m_commandLists[m_currentContextIndex]->DrawIndexedInstanced(defaultQuadMesh->GetElementCount(), 1, 0, 0, 0);
	//m_commandLists[m_currentContextIndex]->DrawInstanced(4, 1, 0, 0);

	auto raytracingOutput = m_raytracingManager->GetRaytracingOutputResource();

	CD3DX12_RESOURCE_BARRIER postCopyBarriers = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
	m_commandLists[m_currentContextIndex]->ResourceBarrier(1, &postCopyBarriers);
}

void Ideal::D3D12RayTracingRenderer::UpdateLightListCBData()
{
	//----------------Directional Light-----------------//
	uint32 directionalLightNum = m_directionalLights.size();
	if (directionalLightNum > MAX_DIRECTIONAL_LIGHT_NUM)
	{
		directionalLightNum = MAX_DIRECTIONAL_LIGHT_NUM;
	}
	for (uint32 i = 0; i < directionalLightNum; ++i)
	{
		m_lightListCB.DirLights[i] = m_directionalLights[i]->GetDirectionalLightDesc();
	}
	m_lightListCB.DirLightNum = directionalLightNum;


	//----------------Spot Light-----------------//
	uint32 spotLightNum = m_spotLights.size();

	if (spotLightNum > MAX_SPOT_LIGHT_NUM)
	{
		spotLightNum = MAX_SPOT_LIGHT_NUM;
	}

	for (uint32 i = 0; i < spotLightNum; ++i)
	{
		m_lightListCB.SpotLights[i] = m_spotLights[i]->GetSpotLightDesc();
	}
	m_lightListCB.SpotLightNum = spotLightNum;

	//----------------Point Light-----------------//
	uint32 pointLightNum = m_pointLights.size();

	if (pointLightNum > MAX_POINT_LIGHT_NUM)
	{
		pointLightNum = MAX_POINT_LIGHT_NUM;
	}
	m_lightListCB.PointLightNum = pointLightNum;

	for (uint32 i = 0; i < pointLightNum; ++i)
	{
		m_lightListCB.PointLights[i] = m_pointLights[i]->GetPointLightDesc();
	}
}

void Ideal::D3D12RayTracingRenderer::CompileDefaultShader()
{
	//---Ray Tracing Shader---//
	CompileShader(L"../Shaders/Raytracing/Raytracing.hlsl",
		L"../Shaders/Raytracing/",
		L"Raytracing",
		L"lib_6_3", L"", L"../Shaders/Raytracing/", false);
	m_raytracingShader = CreateAndLoadShader(L"../Shaders/Raytracing/Raytracing.shader");

	CompileShader(L"../Shaders/Raytracing/CS_ComputeAnimationVertexBuffer.hlsl",
		L"../Shaders/Raytracing/",
		L"CS_ComputeAnimationVertexBuffer",
		L"cs_6_3",
		L"ComputeSkinnedVertex",
		L"../Shaders/Raytracing/",
		true);
	m_animationShader = CreateAndLoadShader(L"../Shaders/Raytracing/CS_ComputeAnimationVertexBuffer.shader");


	//---Debug Mesh Shader---//
	CompileShader(L"../Shaders/DebugMesh/DebugMeshShader.hlsl", L"../Shaders/DebugMesh/", L"DebugMeshShaderVS", L"vs_6_3", L"VSMain");
	CompileShader(L"../Shaders/DebugMesh/DebugMeshShader.hlsl", L"../Shaders/DebugMesh/", L"DebugMeshShaderPS", L"ps_6_3", L"PSMain");
	m_DebugMeshManagerVS = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugMeshShaderVS.shader");
	m_DebugMeshManagerPS = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugMeshShaderPS.shader");

	CompileShader(L"../Shaders/DebugMesh/DebugLineShader.hlsl", L"../Shaders/DebugMesh/", L"DebugLineShaderVS", L"vs_6_3", L"VSMain");
	CompileShader(L"../Shaders/DebugMesh/DebugLineShader.hlsl", L"../Shaders/DebugMesh/", L"DebugLineShaderPS", L"ps_6_3", L"PSMain");

	m_DebugLineShaderVS = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugLineShaderVS.shader");
	m_DebugLineShaderPS = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugLineShaderPS.shader");

	CompileShader(L"../Shaders/Particle/DefaultParticleVS.hlsl", L"../Shaders/Particle/", L"DefaultParticleVS", L"vs_6_3", L"Main", L"../Shaders/Particle/");
	m_DefaultParticleShaderMeshVS = std::static_pointer_cast<Ideal::D3D12Shader>(CreateAndLoadParticleShader(L"DefaultParticleVS"));
}

void Ideal::D3D12RayTracingRenderer::CopyRaytracingOutputToBackBuffer()
{
	ComPtr<ID3D12GraphicsCommandList4> commandlist = m_commandLists[m_currentContextIndex];
	std::shared_ptr<Ideal::D3D12Texture> renderTarget = m_renderTargets[m_frameIndex];
	ComPtr<ID3D12Resource> raytracingOutput = m_raytracingManager->GetRaytracingOutputResource();

	CD3DX12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTarget->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COPY_DEST
	);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	);
	commandlist->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
	commandlist->CopyResource(renderTarget->GetResource(), raytracingOutput.Get());

	CD3DX12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTarget->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

	commandlist->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

void Ideal::D3D12RayTracingRenderer::TransitionRayTracingOutputToRTV()
{
	ComPtr<ID3D12Resource> raytracingOutput = m_raytracingManager->GetRaytracingOutputResource();
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandLists[m_currentContextIndex]->ResourceBarrier(1, &barrier);

}

void Ideal::D3D12RayTracingRenderer::TransitionRayTracingOutputToSRV()
{
	ComPtr<ID3D12Resource> raytracingOutput = m_raytracingManager->GetRaytracingOutputResource();
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_commandLists[m_currentContextIndex]->ResourceBarrier(1, &barrier);

}

void Ideal::D3D12RayTracingRenderer::TransitionRayTracingOutputToUAV()
{
	return;
	ComPtr<ID3D12Resource> raytracingOutput = m_raytracingManager->GetRaytracingOutputResource();
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		raytracingOutput.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
	m_commandLists[m_currentContextIndex]->ResourceBarrier(1, &barrier);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerInit()
{
	m_raytracingManager = std::make_shared<Ideal::RaytracingManager>();
	m_raytracingManager->Init(m_device, m_resourceManager, m_raytracingShader, m_animationShader, m_descriptorManager, m_width, m_height);

	//ResetCommandList();

	m_raytracingManager->FinalCreate2(m_device, m_commandLists[m_currentContextIndex], m_BLASInstancePool[m_currentContextIndex], true);

	//m_commandLists[m_currentContextIndex]->Close();
	//ID3D12CommandList* ppCommandLists[] = { m_commandLists[m_currentContextIndex].Get() };
	//m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//
	//Fence();
	//WaitForFenceValue(m_lastFenceValues[m_currentContextIndex]);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerUpdate()
{
	m_raytracingManager->UpdateMaterial(m_device, m_deferredDeleteManager);
	m_raytracingManager->UpdateTexture(m_device);
	m_raytracingManager->UpdateAccelerationStructures(shared_from_this(), m_device, m_commandLists[m_currentContextIndex], m_BLASInstancePool[m_currentContextIndex], m_deferredDeleteManager);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerAddObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj)
{
	m_raytracingManager->AddObject(
		obj,
		shared_from_this(),
		m_device,
		m_resourceManager,
		m_descriptorManager,
		m_cbAllocator[m_currentContextIndex],
		obj,
		m_deferredDeleteManager,
		obj->GetName().c_str(),
		false
	);
	return;

	//ResetCommandList();
	auto blas = m_raytracingManager->GetBLASByName(obj->GetName().c_str());
	//std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> blas = nullptr;
	bool ShouldBuildShaderTable = true;

	if (blas != nullptr)
	{
		obj->SetBLAS(blas);
		blas->AddRefCount();
		ShouldBuildShaderTable = false;
	}
	else
	{
		// 안에서 add ref count를 실행시키긴 함. ....
		blas = m_raytracingManager->AddBLAS(shared_from_this(), m_device, m_resourceManager, m_descriptorManager, m_cbAllocator[m_currentContextIndex], obj, obj->GetName().c_str(), false);
	}

	if (ShouldBuildShaderTable)
	{
		m_raytracingManager->BuildShaderTables(m_device, m_deferredDeleteManager);
	}

	auto instanceDesc = m_raytracingManager->AllocateInstanceByBLAS(blas);
	obj->SetBLASInstanceDesc(instanceDesc);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerAddBakedObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj)
{
	// 기존과 차이점은 이름으로 부르지 않는다.
	//auto blas = m_raytracingManager->GetBLASByName(obj->GetName().c_str());
	std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> blas;
	bool ShouldBuildShaderTable = true;

	// 안에서 add ref count를 실행시키긴 함. ....
	blas = m_raytracingManager->AddBLAS(shared_from_this(), m_device, m_resourceManager, m_descriptorManager, m_cbAllocator[m_currentContextIndex], obj, obj->GetName().c_str(), false);

	if (ShouldBuildShaderTable)
	{
		m_raytracingManager->BuildShaderTables(m_device, m_deferredDeleteManager);
	}

	auto instanceDesc = m_raytracingManager->AllocateInstanceByBLAS(blas);
	obj->SetBLASInstanceDesc(instanceDesc);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerAddObject(std::shared_ptr<Ideal::IdealSkinnedMeshObject> obj)
{
	//ResetCommandList();
	auto blas = m_raytracingManager->AddBLAS(shared_from_this(), m_device, m_resourceManager, m_descriptorManager, m_cbAllocator[m_currentContextIndex], obj, obj->GetName().c_str(), true);
	// Skinning 데이터는 쉐이더 테이블을 그냥 만든다.
	m_raytracingManager->BuildShaderTables(m_device, m_deferredDeleteManager);

	auto instanceDesc = m_raytracingManager->AllocateInstanceByBLAS(blas);
	obj->SetBLASInstanceDesc(instanceDesc);
}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerDeleteObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj)
{
	const std::wstring& name = obj->GetName();
	auto blas = obj->GetBLAS();
	auto blasInstance = obj->GetBLASInstanceDesc();
	m_raytracingManager->DeleteBLASInstance(blasInstance);
	if (blas != nullptr)
	{
		bool ShouldDeleteBLAS = blas->SubRefCount();
		if (ShouldDeleteBLAS)
		{
			m_deferredDeleteManager->AddBLASToDeferredDelete(blas);
			m_raytracingManager->DeleteBLAS(blas, name, false);
			m_raytracingManager->BuildShaderTables(m_device, m_deferredDeleteManager);
			// HitContributionToIndex 이거 갱신해줘야 한다.

		}
	}
	//obj.reset();

}

void Ideal::D3D12RayTracingRenderer::RaytracingManagerDeleteObject(std::shared_ptr<Ideal::IdealSkinnedMeshObject> obj)
{
	const std::wstring& name = obj->GetName();
	auto blas = obj->GetBLAS();
	//auto blas = m_raytracingManager->GetBLASByName(name);
	auto blasInstance = obj->GetBLASInstanceDesc();
	m_raytracingManager->DeleteBLASInstance(blasInstance);
	if (blas != nullptr)
	{
		bool ShouldDeleteBLAS = blas->SubRefCount();
		if (ShouldDeleteBLAS)
		{
			m_deferredDeleteManager->AddBLASToDeferredDelete(blas);
			m_raytracingManager->DeleteBLAS(blas, name, true);
			m_raytracingManager->BuildShaderTables(m_device, m_deferredDeleteManager);

			//obj->GetBLASInstanceDesc()->InstanceDesc.InstanceContributionToHitGroupIndex = obj->GetBLAS()->GetInstanceContributionToHitGroupIndex();
		}
	}
	//obj.reset();
}

void Ideal::D3D12RayTracingRenderer::CreateUIDescriptorHeap()
{
	//------UI Descriptor Heap------//
	for (uint32 i = 0; i < MAX_PENDING_FRAME_COUNT; ++i)
	{
		m_mainDescriptorHeaps[i] = std::make_shared<Ideal::D3D12DescriptorHeap>();
		m_mainDescriptorHeaps[i]->Create(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, MAX_DESCRIPTOR_COUNT);
	}
}

void Ideal::D3D12RayTracingRenderer::CreateCanvas()
{
	m_UICanvas = std::make_shared<Ideal::IdealCanvas>();
	m_UICanvas->Init(m_device);
	m_UICanvas->SetCanvasSize(m_width, m_height);
}

void Ideal::D3D12RayTracingRenderer::DrawCanvas()
{
	ID3D12DescriptorHeap* descriptorHeap[] = { m_mainDescriptorHeaps[m_currentContextIndex]->GetDescriptorHeap().Get() };
	m_commandLists[m_currentContextIndex]->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

	std::shared_ptr<Ideal::D3D12Texture> renderTarget = m_renderTargets[m_frameIndex];

	m_commandLists[m_currentContextIndex]->RSSetViewports(1, &m_viewport->GetViewport());
	m_commandLists[m_currentContextIndex]->RSSetScissorRects(1, &m_viewport->GetScissorRect());
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
#ifdef BeforeRefactor
	m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &renderTarget->GetRTV().GetCpuHandle(), FALSE, &dsvHandle);
#else
	m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &m_raytracingManager->GetRaytracingOutputRTVHandle().GetCpuHandle(), FALSE, &dsvHandle);
#endif
	m_UICanvas->DrawCanvas(m_device, m_commandLists[m_currentContextIndex], m_mainDescriptorHeaps[m_currentContextIndex], m_cbAllocator[m_currentContextIndex]);
}

void Ideal::D3D12RayTracingRenderer::SetCanvasSize(uint32 Width, uint32 Height)
{
	m_UICanvas->SetCanvasSize(Width, Height);
}

void Ideal::D3D12RayTracingRenderer::UpdateTextureWithImage(std::shared_ptr<Ideal::D3D12Texture> Texture, BYTE* SrcBits, uint32 SrcWidth, uint32 SrcHeight)
{
	if (SrcWidth > Texture->GetWidth())
	{
		__debugbreak();
	}
	if (SrcHeight > Texture->GetHeight())
	{
		__debugbreak();
	}
	D3D12_RESOURCE_DESC desc = Texture->GetResource()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
	uint32 rows = 0;
	uint64 rowSize = 0;
	uint64 TotalBytes = 0;
	m_device->GetCopyableFootprints(&desc, 0, 1, 0, &Footprint, &rows, &rowSize, &TotalBytes);

	BYTE* mappedPtr = nullptr;
	CD3DX12_RANGE writeRange(0, 0);
	HRESULT hr = Texture->GetUploadBuffer()->Map(0, &writeRange, reinterpret_cast<void**>(&mappedPtr));
	Check(hr);

	BYTE* pSrc = SrcBits;
	BYTE* pDest = mappedPtr;
	for (uint32 y = 0; y < SrcHeight; ++y)
	{
		memcpy(pDest, pSrc, SrcWidth * 4);
		pSrc += (SrcWidth * 4);
		pDest += Footprint.Footprint.RowPitch;
	}
	Texture->GetUploadBuffer()->Unmap(0, nullptr);
	Texture->SetUpdated();
}

void Ideal::D3D12RayTracingRenderer::CreateParticleSystemManager()
{
	m_particleSystemManager = std::make_shared<Ideal::ParticleSystemManager>();

	//---VertexShader---//
	// TEMP: 일단 그냥 여기서 컴파일 한다.
	//CompileShader(L"../Shaders/Particle/DefaultParticleVS.hlsl", L"DefaultParticleVS", L"vs_6_3", L"Main", L"../Shaders/Particle/");
	//CompileShader(L"../Shaders/Particle/DefaultParticleVS.hlsl", L"DefaultParticleVS", L"vs_6_3", L"Main", L"../Shaders/Particle/");
	CompileShader(L"../Shaders/Particle/DefaultParticleVS.hlsl", L"../Shaders/Particle/", L"DefaultParticleVS", L"vs_6_3", L"Main", L"../Shaders/Particle/");
	auto defaultParticleMeshVS = CreateAndLoadParticleShader(L"DefaultParticleVS");

	CompileShader(L"../Shaders/Particle/DefaultParticleBillboardShader.hlsl", L"../Shaders/Particle/", L"DefaultParticleBillboardShaderVS", L"vs_6_3", L"VSMain", L"../Shaders/Particle/");
	auto defaultParticleBillboardVS = CreateAndLoadParticleShader(L"DefaultParticleBillboardShaderVS");

	CompileShader(L"../Shaders/Particle/DefaultParticleBillboardShader.hlsl", L"../Shaders/Particle/", L"DefaultParticleBillboardShaderGS", L"gs_6_3", L"GSMain", L"../Shaders/Particle/");
	auto defaultParticleBillboardGS = CreateAndLoadParticleShader(L"DefaultParticleBillboardShaderGS");

	CompileShader(L"../Shaders/Particle/CS_DefaultParticleBillboard.hlsl", L"../Shaders/Particle/", L"DefaultParticleBillboardShaderCS", L"cs_6_3", L"CSMain", L"../Shaders/Particle");
	//auto defaultParticleBillboardCS = CreateAndLoadParticleShader(L"DefaultParticleBillboardShaderCS");
	m_DefaultParticleShaderBillboardCS = std::static_pointer_cast<Ideal::D3D12Shader>(CreateAndLoadParticleShader(L"DefaultParticleBillboardShaderCS"));

	//---Init---//
	m_particleSystemManager->Init(m_device);
	m_particleSystemManager->SetMeshVS(std::static_pointer_cast<Ideal::D3D12Shader>(defaultParticleMeshVS));
	m_particleSystemManager->SetBillboardVS(std::static_pointer_cast<Ideal::D3D12Shader>(defaultParticleBillboardVS));
	m_particleSystemManager->SetBillboardGS(std::static_pointer_cast<Ideal::D3D12Shader>(defaultParticleBillboardGS));
	m_particleSystemManager->SetBillboardCSAndCreatePipelineState(m_DefaultParticleShaderBillboardCS, m_device);
	m_particleSystemManager->SetDefaultParticleVertexBuffer(m_resourceManager->GetParticleVertexBuffer());
}

void Ideal::D3D12RayTracingRenderer::DrawParticle()
{
	ID3D12DescriptorHeap* descriptorHeap[] = { m_mainDescriptorHeaps[m_currentContextIndex]->GetDescriptorHeap().Get() };
	m_commandLists[m_currentContextIndex]->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

	std::shared_ptr<Ideal::D3D12Texture> renderTarget = m_renderTargets[m_frameIndex];

	m_commandLists[m_currentContextIndex]->RSSetViewports(1, &m_viewport->GetViewport());
	m_commandLists[m_currentContextIndex]->RSSetScissorRects(1, &m_viewport->GetScissorRect());
	//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetGPUDescriptorHandleForHeapStart());
	// TODO : DSV SET
	auto depthBuffer = m_raytracingManager->GetDepthBuffer();
	//m_commandLists[m_currentContextIndex]->OMSetRenderTargets()
	//m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &m_raytracingManager->GetRaytracingOutputRTVHandle().GetCpuHandle(), FALSE, &depthBuffer->GetDSV().GetCpuHandle());
	m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &m_raytracingManager->GetRaytracingOutputRTVHandle().GetCpuHandle(), FALSE, &depthBuffer->GetDSV().GetCpuHandle());
	//m_commandLists[m_currentContextIndex]->OMSetRenderTargets(1, &m_raytracingManager->GetRaytracingOutputRTVHandle().GetCpuHandle(), FALSE, NULL);
	m_particleSystemManager->DrawParticles(m_device, m_commandLists[m_currentContextIndex], m_mainDescriptorHeaps[m_currentContextIndex], m_cbAllocator[m_currentContextIndex], &m_globalCB, m_mainCamera);
}

void Ideal::D3D12RayTracingRenderer::CreateDebugMeshManager()
{
	m_debugMeshManager = std::make_shared<Ideal::DebugMeshManager>();

	//CompileShader(L"../Shaders/DebugMesh/DebugMeshShader.hlsl", L"../Shaders/DebugMesh/", L"DebugMeshShaderVS", L"vs_6_3", L"VSMain");
	//CompileShader(L"../Shaders/DebugMesh/DebugMeshShader.hlsl", L"../Shaders/DebugMesh/", L"DebugMeshShaderPS", L"ps_6_3", L"PSMain");
	//
	//auto vs = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugMeshShaderVS.shader");
	//auto ps = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugMeshShaderPS.shader");

	m_debugMeshManager->SetVS(m_DebugMeshManagerVS);
	m_debugMeshManager->SetPS(m_DebugMeshManagerPS);

	//CompileShader(L"../Shaders/DebugMesh/DebugLineShader.hlsl", L"../Shaders/DebugMesh/", L"DebugLineShaderVS", L"vs_6_3", L"VSMain");
	//CompileShader(L"../Shaders/DebugMesh/DebugLineShader.hlsl", L"../Shaders/DebugMesh/", L"DebugLineShaderPS", L"ps_6_3", L"PSMain");
	//
	//auto vsLine = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugLineShaderVS.shader");
	//auto psLine = CreateAndLoadShader(L"../Shaders/DebugMesh/DebugLineShaderPS.shader");

	m_debugMeshManager->SetVSLine(m_DebugLineShaderVS);
	m_debugMeshManager->SetPSLine(m_DebugLineShaderPS);

	m_debugMeshManager->SetDebugLineVB(m_resourceManager->GetDebugLineVB());

	m_debugMeshManager->Init(m_device);
}

void Ideal::D3D12RayTracingRenderer::DrawDebugMeshes()
{
	if (m_isEditor)
	{
		m_debugMeshManager->DrawDebugMeshes(m_device, m_commandLists[m_currentContextIndex], m_mainDescriptorHeaps[m_currentContextIndex], m_cbAllocator[m_currentContextIndex], &m_globalCB);
	}
}

void Ideal::D3D12RayTracingRenderer::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	//-----Allocate Srv-----//
	m_imguiSRVHandle = m_imguiSRVHeap->Allocate();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hwnd);
	ImGui_ImplDX12_Init(m_device.Get(), SWAP_CHAIN_FRAME_COUNT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_imguiSRVHeap->GetDescriptorHeap().Get(),
		m_imguiSRVHandle.GetCpuHandle(),
		m_imguiSRVHandle.GetGpuHandle());
}

void Ideal::D3D12RayTracingRenderer::DrawImGuiMainCamera()
{
	ImGui::Begin("MAIN SCREEN", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);		// Create a window called "Hello, world!" and append into it.

	ImVec2 windowPos = ImGui::GetWindowPos(); // 현재 윈도우 포지션
	ImVec2 min = ImGui::GetWindowContentRegionMin(); // 컨텐츠 포지션 왼쪽 위 -> 윈도우 왼쪽 위 기준
	ImVec2 max = ImGui::GetWindowContentRegionMax(); // 컨텐츠 포지션 오른쪽 아래 -> 윈도우 왼쪽 위 기준
	ImVec2 windowSize = ImGui::GetWindowSize();
	auto a = ImGui::GetWindowHeight();
	auto b = ImGui::GetWindowWidth();
	ImVec2 size(windowSize.x, windowSize.y);


	m_mainCameraEditorTopLeft.x = windowPos.x + min.x;
	m_mainCameraEditorTopLeft.y = windowPos.y + min.y;

	//m_mainCameraEditorBottomRight.x = windowPos.x + max.x;
	//m_mainCameraEditorBottomRight.y = windowPos.y + max.y;

	float viewWidthRatio = static_cast<float>(m_width) / windowSize.x;
	float viewHeightRatio = static_cast<float>(m_height) / windowSize.y;

	float x = 1.0f;
	float y = 1.0f;

	if (viewWidthRatio < viewHeightRatio)
	{
		// The scaled image's height will fit to the viewport's height and 
		// its width will be smaller than the viewport's width.
		x = viewWidthRatio / viewHeightRatio;
	}
	else
	{
		// The scaled image's width will fit to the viewport's width and 
		// its height may be smaller than the viewport's height.
		y = viewHeightRatio / viewWidthRatio;
	}
	size.x = x * windowSize.x;
	size.y = y * windowSize.y;
	m_mainCameraEditorBottomRight.x = windowPos.x + size.x;
	m_mainCameraEditorBottomRight.y = windowPos.y + min.y + size.y;
	m_mainCameraEditorWindowSize.x = size.x - min.x;
	m_mainCameraEditorWindowSize.y = size.y - min.y;

	ImGui::Image((ImTextureID)(m_editorTexture->GetSRV().GetGpuHandle().ptr), size);
	ImGui::End();
}

void Ideal::D3D12RayTracingRenderer::SetImGuiCameraRenderTarget()
{
	ComPtr<ID3D12CommandAllocator> commandAllocator = m_commandAllocators[m_currentContextIndex];
	ComPtr<ID3D12GraphicsCommandList> commandList = m_commandLists[m_currentContextIndex];

	CD3DX12_RESOURCE_BARRIER editorBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_editorTexture->GetResource(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList->ResourceBarrier(1, &editorBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	auto rtvHandle = m_editorTexture->GetRTV();


	commandList->ClearRenderTargetView(rtvHandle.GetCpuHandle(), DirectX::Colors::Black, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &m_viewport->GetViewport());
	commandList->RSSetScissorRects(1, &m_viewport->GetScissorRect());
	//commandList->RSSetViewports(1, &m_postViewport->GetViewport());
	//commandList->RSSetScissorRects(1, &m_viewport->GetScissorRect());
	commandList->OMSetRenderTargets(1, &rtvHandle.GetCpuHandle(), FALSE, &dsvHandle);
}

void Ideal::D3D12RayTracingRenderer::CreateEditorRTV(uint32 Width, uint32 Height)
{
	m_editorTexture = std::make_shared<Ideal::D3D12Texture>();
	{
		{
			ComPtr<ID3D12Resource> resource;
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_R8G8B8A8_UNORM,
				Width,
				Height, 1, 1
			);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			//------Create------//
			Check(m_device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				//D3D12_RESOURCE_STATE_COMMON,
				//nullptr,
				nullptr,
				IID_PPV_ARGS(resource.GetAddressOf())
			));
			m_editorTexture->Create(resource, m_deferredDeleteManager);

			//-----SRV-----//
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = resource->GetDesc().Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;
				Ideal::D3D12DescriptorHandle srvHandle = m_imguiSRVHeap->Allocate();
				m_device->CreateShaderResourceView(resource.Get(), &srvDesc, srvHandle.GetCpuHandle());
				m_editorTexture->EmplaceSRV(srvHandle);
			}
			//-----RTV-----//
			{
				D3D12_RENDER_TARGET_VIEW_DESC RTVDesc{};
				RTVDesc.Format = resource->GetDesc().Format;
				RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				RTVDesc.Texture2D.MipSlice = 0;

				Ideal::D3D12DescriptorHandle rtvHandle = m_editorRTVHeap->Allocate();
				m_device->CreateRenderTargetView(resource.Get(), &RTVDesc, rtvHandle.GetCpuHandle());
				m_editorTexture->EmplaceRTV(rtvHandle);
			}
			m_editorTexture->GetResource()->SetName(L"Editor Texture");
		}
	}
}
