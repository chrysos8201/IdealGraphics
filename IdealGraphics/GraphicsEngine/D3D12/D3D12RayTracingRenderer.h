#pragma once

#include "Core/Core.h"
#include "GraphicsEngine/public/IdealRenderer.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"

#include <d3d12.h>
#include <d3dx12.h>

#include "GraphicsEngine/D3D12/D3D12Definitions.h"

#include "Misc/Utils/Octree/Octree.h"

// Test
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/VertexInfo.h"
#include <dxgi.h>
#include <dxgi1_6.h>

#include <dxcapi.h>
//#include <atlbase.h>

struct ID3D12Device;
struct ID3D12CommandQueue;
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

namespace Ideal
{
	class ResourceManager;
	class ShaderManager;

	class D3D12DescriptorHeap;
	class D3D12DynamicDescriptorHeap;
	class D3D12Texture;
	class D3D12PipelineStateObject;
	class D3D12Viewport;
	class D3D12UAVBuffer;
	class D3D12StructuredBuffer;

	class IdealCamera;
	class IdealRenderScene;
	class IdealRaytracingRenderScene;
	class IdealStaticMeshObject;
	class IdealSkinnedMeshObject;
	class IdealScreenQuad;
	class IdealDirectionalLight;
	class IdealSpotLight;
	class IdealPointLight;
	class IdealCanvas;
	class IdealSprite;

	class ParticleSystemManager;
	class ParticleSystem;
	class ParticleMaterial;

	struct ConstantBufferContainer;
	// Manager
	class D3D12ConstantBufferPool;
	class D3D12DynamicConstantBufferAllocator;
	class D3D12UploadBufferPool;
	class DeferredDeleteManager;
	class DebugMeshManager;

	// Interface
	class ICamera;
	class IRenderScene;
	class IMeshObject;
	class ISkinnedMeshObject;
	class IRenderScene;
	class IText;
	class ISprite;


	// TEST : DELETE
	template<typename>
	class IdealMesh;
	class D3D12VertexBuffer;
	class D3D12IndexBuffer;
	class D3D12Shader;
	class D3D12UAVBuffer;
	class D3D12ShaderResourceView;

	class DXRAccelerationStructureManager;
	class RaytracingManager;
	class D3D12DescriptorManager;

	class D2DTextManager;
	struct FontHandle;
	class IdealText;
}

namespace Ideal
{
	class D3D12RayTracingRenderer : public Ideal::IdealRenderer, public std::enable_shared_from_this<D3D12RayTracingRenderer>
	{
	public:
		static const uint32 SWAP_CHAIN_FRAME_COUNT = G_SWAP_CHAIN_NUM;
		static const uint32 MAX_PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;

	private:
		//static const uint32 MAX_DRAW_COUNT_PER_FRAME = 8192;
		static const uint32 MAX_DRAW_COUNT_PER_FRAME = 16384;
		static const uint32	MAX_DESCRIPTOR_COUNT = 16384 * 4;
		static const uint32	MAX_UI_DESCRIPTOR_COUNT = 256;
		static const uint32 MAX_EDITOR_SRV_COUNT = 16384;

		// Display Resolution
		static const DisplayResolution m_resolutionOptions[];

	public:
		D3D12RayTracingRenderer(HWND hwnd, uint32 Width, uint32 Height, bool EditorMode);
		virtual ~D3D12RayTracingRenderer();

	public:
		void Init() override;
		void Tick() override;
		void Render() override;
		void Resize(UINT Width, UINT Height) override;
		void ToggleFullScreenWindow() override;
		bool IsFullScreen() override;
		void SetDisplayResolutionOption(const Resolution::EDisplayResolutionOption& Resolution) override;

		std::shared_ptr<ICamera> CreateCamera() override;
		void SetMainCamera(std::shared_ptr<ICamera> Camera) override;
		std::shared_ptr<Ideal::IMeshObject> CreateStaticMeshObject(const std::wstring& FileName) override;
		std::shared_ptr<Ideal::ISkinnedMeshObject> CreateSkinnedMeshObject(const std::wstring& FileName) override;
		void DeleteMeshObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) override;
		void DeleteDebugMeshObject(std::shared_ptr<Ideal::IMeshObject> DebugMeshObject) override;
		// 작동 안함 // 그냥 mesh object 반환
		virtual std::shared_ptr<Ideal::IMeshObject>	CreateDebugMeshObject(const std::wstring& FileName) override;

		std::shared_ptr<Ideal::IAnimation> CreateAnimation(const std::wstring& FileName, const Matrix& offset = Matrix::Identity) override;
		std::shared_ptr<Ideal::IDirectionalLight> CreateDirectionalLight() override;
		std::shared_ptr<Ideal::ISpotLight> CreateSpotLight() override;
		std::shared_ptr<Ideal::IPointLight> CreatePointLight() override;
		void DeleteLight(std::shared_ptr<Ideal::ILight> Light) override;


		void SetAssetPath(const std::wstring& AssetPath) override;
		void SetModelPath(const std::wstring& ModelPath) override;
		void SetTexturePath(const std::wstring& TexturePath) override;
		void ConvertAssetToMyFormat(std::wstring FileName, bool isSkinnedData = false, bool NeedVertexInfo = false, bool NeedConvertCenter = false) override;
		void ConvertAnimationAssetToMyFormat(std::wstring FileName) override;
		void ConvertParticleMeshAssetToMyFormat(std::wstring FileName, bool SetScale = false, Vector3 Scale = Vector3(1.f)) override;
		bool SetImGuiWin32WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
		void ClearImGui() override;

		DirectX::SimpleMath::Vector2 GetTopLeftEditorPos() override;
		DirectX::SimpleMath::Vector2 GetRightBottomEditorPos() override;

		virtual void SetSkyBox(std::shared_ptr<Ideal::ITexture> SkyBoxTexture) override;
		virtual std::shared_ptr<Ideal::ITexture> CreateSkyBox(const std::wstring& FileName) override;

		// Texture
		virtual std::shared_ptr<Ideal::ITexture> CreateTexture(const std::wstring& FileName, bool IsGenerateMips = false, bool IsNormalMap = false, bool IngoreSRGB = false) override;
		virtual std::shared_ptr<Ideal::IMaterial> CreateMaterial() override;

		virtual void DeleteTexture(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void DeleteMaterial(std::shared_ptr<Ideal::IMaterial> Material) override;

		// Sprite
		virtual std::shared_ptr<Ideal::ISprite> CreateSprite() override;
		virtual void DeleteSprite(std::shared_ptr<Ideal::ISprite>& Sprite) override;

		// Text
		virtual std::shared_ptr<Ideal::IText> CreateText(uint32 Width, uint32 Height, float FontSize, std::wstring Font = L"Tahoma") override;
		virtual void DeleteText(std::shared_ptr<Ideal::IText>& Text) override;

		// Shader
		virtual void CompileShader(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName, const std::wstring& ShaderVersion, const std::wstring& EntryPoint = L"Main", const std::wstring& IncludeFilePath = L"", bool HasEntry = true) override;	// 셰이더를 컴파일하여 저장. 한 번만 하면 됨.
		virtual std::shared_ptr<Ideal::IShader> CreateAndLoadParticleShader(const std::wstring& Name) override;
		std::shared_ptr<Ideal::D3D12Shader> CreateAndLoadShader(const std::wstring& FilePath);

		// Particle
		virtual std::shared_ptr<Ideal::IParticleSystem> CreateParticleSystem(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial) override;
		virtual void DeleteParticleSystem(std::shared_ptr<Ideal::IParticleSystem>& ParticleSystem) override;

		// ParticleMaterial
		virtual std::shared_ptr<Ideal::IParticleMaterial> CreateParticleMaterial() override;
		virtual void DeleteParticleMaterial(std::shared_ptr<Ideal::IParticleMaterial>& ParticleMaterial) override;

		// ParticleMesh
		virtual std::shared_ptr<Ideal::IMesh> CreateParticleMesh(const std::wstring& FileName) override;

		virtual void SetRendererAmbientIntensity(float Value) override;

	private:
		void CreateCommandlists();
		void CreatePools();
		void CreateSwapChains(ComPtr<IDXGIFactory6> Factory);
		void CreateRTV();
		void CreateDSVHeap();
		void CreateDSV(uint32 Width, uint32 Height);
		void CreateFence();

	private:
		void CreateDefaultCamera();
		void UpdatePostViewAndScissor(uint32 Width, uint32 Height);

	private:
		void ResetCommandList();
		void BeginRender();
		void EndRender();
		void Present();

	public:
		uint64 Fence();
		void WaitForFenceValue(uint64 ExpectedFenceValue);

	private:
		ComPtr<ID3D12Device5> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;

		ComPtr<ID3D12GraphicsCommandList4> m_commandLists[MAX_PENDING_FRAME_COUNT];
		ComPtr<ID3D12CommandAllocator> m_commandAllocators[MAX_PENDING_FRAME_COUNT];
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_descriptorHeaps[MAX_PENDING_FRAME_COUNT] = {};
		std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> m_cbAllocator[MAX_PENDING_FRAME_COUNT] = {};
		std::shared_ptr<Ideal::D3D12UploadBufferPool> m_BLASInstancePool[MAX_PENDING_FRAME_COUNT] = {};

	private:
		void CreatePostScreenRootSignature();
		void CreatePostScreenPipelineState();
		void DrawPostScreen();

	private:
		uint32 m_width = 0;
		uint32 m_height = 0;
		Ideal::Resolution::EDisplayResolutionOption m_displayResolutionIndex = Ideal::Resolution::EDisplayResolutionOption::R_800_600;
		HWND m_hwnd;

		std::shared_ptr<Ideal::D3D12Viewport> m_viewport = nullptr;
		std::shared_ptr<Ideal::D3D12Viewport> m_postViewport = nullptr;
		ComPtr<ID3D12RootSignature> m_postScreenRootSignature;
		ComPtr<ID3D12PipelineState> m_postScreenPipelineState;

	public:
		uint64 m_lastFenceValues[MAX_PENDING_FRAME_COUNT] = {};
		uint64 m_currentContextIndex = 0;

	private:
		ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
		UINT m_swapChainFlags;
		uint32 m_frameIndex = 0;
		BOOL m_windowMode = TRUE;
		bool m_tearingSupport = true;
		bool m_fullScreenMode = false;
		RECT m_windowRect;

		// RTV
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_rtvHeap;
		std::shared_ptr<Ideal::D3D12Texture> m_renderTargets[SWAP_CHAIN_FRAME_COUNT];
		// DSV
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
		ComPtr<ID3D12Resource> m_depthStencil = nullptr;

		ComPtr<ID3D12Fence> m_fence = nullptr;
		uint64 m_fenceValue = 0;
		HANDLE m_fenceEvent = NULL;

	private:
		std::wstring m_assetPath;
		std::wstring m_modelPath;
		std::wstring m_texturePath;

	private:
		// Main Camera
		std::shared_ptr<Ideal::IdealCamera> m_mainCamera = nullptr;
		float m_aspectRatio = 0.f;

		// sky box
		std::shared_ptr<Ideal::D3D12Texture> m_skyBoxTexture = nullptr;

		// Manager
		std::shared_ptr<Ideal::ResourceManager> m_resourceManager = nullptr;
		std::shared_ptr<Ideal::DeferredDeleteManager> m_deferredDeleteManager = nullptr;

		// Text
		std::shared_ptr<Ideal::D2DTextManager> m_textManager;

		// Light
		void UpdateLightListCBData();

		std::vector<std::shared_ptr<Ideal::IdealDirectionalLight>> m_directionalLights;
		std::vector<std::shared_ptr<Ideal::IdealSpotLight>> m_spotLights;
		std::vector<std::shared_ptr<Ideal::IdealPointLight>> m_pointLights;

		// InitShaders
	public:
		void CompileDefaultShader();

		// Shaders
	private:
		std::shared_ptr<Ideal::D3D12Shader> m_raytracingShader;
		std::shared_ptr<Ideal::D3D12Shader> m_animationShader;
		std::shared_ptr<Ideal::D3D12Shader> m_DebugMeshManagerVS;
		std::shared_ptr<Ideal::D3D12Shader> m_DebugMeshManagerPS;
		std::shared_ptr<Ideal::D3D12Shader> m_DebugLineShaderVS;
		std::shared_ptr<Ideal::D3D12Shader> m_DebugLineShaderPS;
		std::shared_ptr<Ideal::D3D12Shader> m_DefaultParticleShaderMeshVS;
		std::shared_ptr<Ideal::D3D12Shader> m_DefaultParticleShaderBillboardCS;
		// RAY TRACING FRAMEWORK

	private:
		// AS
		SceneConstantBuffer m_sceneCB;
		CB_LightList m_lightListCB;
		CB_Global m_globalCB;

		// Render
		void CopyRaytracingOutputToBackBuffer();
		void TransitionRayTracingOutputToRTV();
		void TransitionRayTracingOutputToSRV();
		void TransitionRayTracingOutputToUAV();

		// AS Manager	
		std::shared_ptr<Ideal::RaytracingManager> m_raytracingManager;
		std::vector<std::shared_ptr<Ideal::IdealStaticMeshObject>> m_staticMeshObject;
		std::vector<std::shared_ptr<Ideal::IdealSkinnedMeshObject>> m_skinnedMeshObject;

		// 여러 Descriptor Pool과 고정적으로 사용될 srv_cbv_uav heap을 합쳤다.
		std::shared_ptr<Ideal::D3D12DescriptorManager> m_descriptorManager;

		// 2024.07.02 Wait 뺀 버전의 BLAS , TLAS 빌드 만들기
		void RaytracingManagerInit();
		void RaytracingManagerUpdate();
		void RaytracingManagerAddObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj);
		void RaytracingManagerAddBakedObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj);
		void RaytracingManagerAddObject(std::shared_ptr<Ideal::IdealSkinnedMeshObject> obj);

		void RaytracingManagerDeleteObject(std::shared_ptr<Ideal::IdealStaticMeshObject> obj);
		void RaytracingManagerDeleteObject(std::shared_ptr<Ideal::IdealSkinnedMeshObject> obj);

		// UI
	private:
		void CreateUIDescriptorHeap();
		void CreateCanvas();
		void DrawCanvas();
		void SetCanvasSize(uint32 Width, uint32 Height);
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_mainDescriptorHeaps[MAX_PENDING_FRAME_COUNT];
		std::shared_ptr<Ideal::IdealCanvas> m_UICanvas;

		void UpdateTextureWithImage(std::shared_ptr<Ideal::D3D12Texture> Texture, BYTE* SrcBits, uint32 SrcWidth, uint32 SrcHeight);

		// Particle
	private:
		void CreateParticleSystemManager();
		void DrawParticle();

		std::shared_ptr<Ideal::ParticleSystemManager> m_particleSystemManager;

		// DebugMesh
	private:
		void CreateDebugMeshManager();
		void DrawDebugMeshes();

		std::shared_ptr<Ideal::DebugMeshManager> m_debugMeshManager;

		// EDITOR 
	private:
		void InitImGui();
		void DrawImGuiMainCamera();
		void SetImGuiCameraRenderTarget();
		void CreateEditorRTV(uint32 Width, uint32 Height);

		bool m_isEditor;
		Ideal::D3D12DescriptorHandle m_imguiSRVHandle;
		std::shared_ptr<Ideal::D3D12DynamicDescriptorHeap> m_imguiSRVHeap;
		std::shared_ptr<Ideal::D3D12DynamicDescriptorHeap> m_editorRTVHeap;
		std::shared_ptr<Ideal::D3D12Texture> m_editorTexture;

		Vector2 m_mainCameraEditorTopLeft;
		Vector2 m_mainCameraEditorBottomRight;
		Vector2 m_mainCameraEditorWindowSize;

		// Simple Terrain
	private:
		void InitTerrain();
		void LoadRawHeightMap();
		void SetTerrainCoordinates();
		void CalculateNormals();
		void LoadColorMap();
		void DrawTerrain();

		// build to ray tracing
		void AddTerrainToRaytracing();

		std::shared_ptr<Ideal::D3D12VertexBuffer> m_terrainVB;
		std::shared_ptr<Ideal::D3D12IndexBuffer> m_terrainIB;
		uint32 m_terrainWidth = 1025;
		uint32 m_terrainHeight = 1025;
		ComPtr<ID3D12RootSignature> m_terrainRootSignature;
		ComPtr<ID3D12PipelineState> m_terrainPipelineState;
		std::shared_ptr<Ideal::D3D12Shader> m_terrainVS;
		std::shared_ptr<Ideal::D3D12Shader> m_terrainPS;
		Matrix m_terrainTransform;
		std::vector<BasicVertex> m_heightMap;

		std::shared_ptr<Ideal::IdealStaticMeshObject> m_terrainMesh;

		// Simple Tessellation
	private:
		void InitTessellation();
		void CompileShaderTessellation();
		void DrawTessellation();

		ComPtr<ID3D12RootSignature> m_simpleTessellationRootSignature;
		ComPtr<ID3D12PipelineState> m_simpleTessellationPipelineState;

		std::shared_ptr<Ideal::D3D12VertexBuffer> m_simpleTessellationVB;
		std::shared_ptr<Ideal::D3D12Shader> m_simpleTessellationVS;
		std::shared_ptr<Ideal::D3D12Shader> m_simpleTessellationHS;
		std::shared_ptr<Ideal::D3D12Shader> m_simpleTessellationPS;
		std::shared_ptr<Ideal::D3D12Shader> m_simpleTessellationDS;

		Matrix m_simpleTessellationTransform;
	};
}
