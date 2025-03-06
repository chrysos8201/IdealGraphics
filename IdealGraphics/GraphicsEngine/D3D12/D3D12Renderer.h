#pragma once

// Main Interface
#include "Core/Core.h"
#include "GraphicsEngine/public/IdealRenderer.h"


#include <d3d12.h>
//#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include <GraphicsEngine/D3D12/D3D12Definitions.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

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

	class D3D12DescriptorHeap;
	class D3D12DynamicDescriptorHeap;
	class D3D12Texture;
	class D3D12PipelineStateObject;
	class D3D12Viewport;

	class IdealCamera;
	class IdealRenderScene;
	class IdealStaticMeshObject;
	class IdealSkinnedMeshObject;
	class IdealScreenQuad;

	struct ConstantBufferContainer;
	// Manager
	class D3D12ConstantBufferPool;
	class D3D12DynamicConstantBufferAllocator;
	class DeferredDeleteManager;

	// Interface
	class ICamera;
	class IRenderScene;
	class IMeshObject;
	class ISkinnedMeshObject;
	class IRenderScene;
	class IMaterial;
	class IText;
}

namespace Ideal
{
	class D3D12Renderer : public Ideal::IdealRenderer, public std::enable_shared_from_this<D3D12Renderer>
	{
	public:
		static const uint32 SWAP_CHAIN_FRAME_COUNT = G_SWAP_CHAIN_NUM;	// TEMP : 사실 FRAME_BUFFER_COUNT와 동일하다.
		static const uint32 MAX_PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;

	private:
		static const uint32 MAX_DRAW_COUNT_PER_FRAME = 1024;
		static const uint32	MAX_DESCRIPTOR_COUNT = 4096;

	public:
		D3D12Renderer(HWND hwnd, uint32 width, uint32 height, bool EditorMode);
		virtual ~D3D12Renderer();

	public:
		//---------------------Ideal Renderer Interface-----------------------//
		virtual void Init() override;
		virtual void Tick() override;
		virtual void ShutDown() override;
		virtual void Render() override;
		virtual void Resize(UINT Width, UINT Height) override;
		virtual void ToggleFullScreenWindow() override;
		virtual bool IsFullScreen() override;
		virtual void SetDisplayResolutionOption(const Resolution::EDisplayResolutionOption& Resolution) override;

		virtual std::shared_ptr<Ideal::ICamera> CreateCamera() override;
		virtual void SetMainCamera(std::shared_ptr<Ideal::ICamera> Camera) override;

		virtual std::shared_ptr<Ideal::IMeshObject> CreateStaticMeshObject(const std::wstring& FileName) override;
		virtual std::shared_ptr<Ideal::ISkinnedMeshObject> CreateSkinnedMeshObject(const std::wstring& FileName) override;
		virtual std::shared_ptr<Ideal::IAnimation> CreateAnimation(const std::wstring& FileName, const Matrix& _offset = Matrix::Identity) override;

		virtual std::shared_ptr<Ideal::IMeshObject>	CreateDebugMeshObject(const std::wstring& FileName) override;

		virtual void DeleteMeshObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) override;
		virtual void DeleteDebugMeshObject(std::shared_ptr<Ideal::IMeshObject> DebugMeshObject) override;

		std::shared_ptr<Ideal::IRenderScene> CreateRenderScene();
		void SetRenderScene(std::shared_ptr<Ideal::IRenderScene> RenderScene);

		virtual std::shared_ptr<Ideal::IDirectionalLight>	CreateDirectionalLight() override;
		virtual std::shared_ptr<Ideal::ISpotLight>			CreateSpotLight() override;
		virtual std::shared_ptr<Ideal::IPointLight>			CreatePointLight() override;

		virtual void DeleteLight(std::shared_ptr<Ideal::ILight> Light) override;

		virtual void SetSkyBox(std::shared_ptr<Ideal::ITexture> SkyBoxTexture) override;
		virtual std::shared_ptr<Ideal::ITexture> CreateSkyBox(const std::wstring& FileName) override;

		virtual std::shared_ptr<Ideal::ITexture> CreateTexture(const std::wstring& FileName, bool IsGenerateMips = false, bool IsNormalMap = false, bool IgnoreSRGB = false) override;
		virtual std::shared_ptr<Ideal::IMaterial> CreateMaterial() override;

		virtual void DeleteTexture(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void DeleteMaterial(std::shared_ptr<Ideal::IMaterial> Material) override;

		// Sprite
		virtual std::shared_ptr<Ideal::ISprite> CreateSprite() override;
		virtual void DeleteSprite(std::shared_ptr<Ideal::ISprite>& Sprite) override;
		virtual std::shared_ptr<Ideal::IText> CreateText(uint32 Width, uint32 Height, float FontSize, std::wstring Font = L"Tahoma") override;
		virtual void DeleteText(std::shared_ptr<Ideal::IText>& Text) override;

		// Shader
		virtual void CompileShader(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName, const std::wstring& ShaderVersion, const std::wstring& EntryPoint = L"Main", const std::wstring& IncludeFilePath = L"", bool HasEntry = true) override;
		virtual std::shared_ptr<Ideal::IShader> CreateAndLoadParticleShader(const std::wstring& Name) override;

		// Particle
		virtual std::shared_ptr<Ideal::IParticleSystem> CreateParticleSystem(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial) override;
		virtual void DeleteParticleSystem(std::shared_ptr<Ideal::IParticleSystem>& ParticleSystem) override;

		// ParticleMaterial
		virtual std::shared_ptr<Ideal::IParticleMaterial> CreateParticleMaterial() override;
		virtual void DeleteParticleMaterial(std::shared_ptr<Ideal::IParticleMaterial>& ParticleMaterial) override;

		// ParticleMesh
		virtual std::shared_ptr<Ideal::IMesh> CreateParticleMesh(const std::wstring& FileName) override;

	public:
		//--------Asset Info---------//
		virtual void SetAssetPath(const std::wstring& AssetPath) override { m_assetPath = AssetPath; }
		virtual void SetModelPath(const std::wstring& ModelPath) override { m_modelPath = ModelPath; }
		virtual void SetTexturePath(const std::wstring& TexturePath) override { m_texturePath = TexturePath; }

		virtual void ConvertAssetToMyFormat(std::wstring FileName, bool isSkinnedData = false, bool NeedVertexInfo = false, bool NeedConvertCenter = false) override;
		virtual void ConvertAnimationAssetToMyFormat(std::wstring FileName) override;
		virtual void ConvertParticleMeshAssetToMyFormat(std::wstring FileName, bool SetScale = false, Vector3 Scale = Vector3(1.f)) override;

		//--------ImGui--------//
		virtual bool SetImGuiWin32WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
		virtual void ClearImGui() override;

		virtual DirectX::SimpleMath::Vector2 GetTopLeftEditorPos() override;
		virtual DirectX::SimpleMath::Vector2 GetRightBottomEditorPos() override;

		virtual void SetRendererAmbientIntensity(float Value) override;

	public:
		void Release();

		//----Create----//
		void CreateAndInitRenderingResources();
		void CreateFence();
		void CreateDSV(uint32 Width, uint32 Height);

		//-------Device------//
		ComPtr<ID3D12Device> GetDevice();

		//------Render-----//
		void ResetCommandList();
		uint32 GetFrameIndex() const;

		void BeginRender();
		void EndRender();
		void Present();

		//------CommandList------//
		ComPtr<ID3D12GraphicsCommandList> GetCommandList();
		ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }

		//------Fence------//
		uint64 Fence();
		void WaitForFenceValue(uint64 ExpectedFenceValue);

		//------Graphics Manager------//
		std::shared_ptr<Ideal::ResourceManager> GetResourceManager();

		// Test : 2024.05.07; 하나의 Descriptor Table에서 떼어다가 쓴다...
		// 2024.05.08 : 메인 Descriptor Heap
		void CreateDescriptorHeap();
		std::shared_ptr<Ideal::D3D12DescriptorHeap> GetMainDescriptorHeap();

		// 2024.05.31 : Dynamic ConstantBuffer Allocator 에서 직접 CBContainer를 받아오도록 수정
		std::shared_ptr<Ideal::ConstantBufferContainer> ConstantBufferAllocate(uint32 SizePerCB);

		//------Camera------//
		void CreateDefaultCamera(); // default camera
		Matrix GetView();
		Matrix GetProj();
		Matrix GetViewProj();
		Vector3 GetEyePos();

		//-----etc-----//
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() { return m_dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
		std::shared_ptr<Ideal::D3D12Viewport> GetViewport() { return m_viewport; }
		std::shared_ptr<Ideal::D3D12DynamicDescriptorHeap> GetImguiSRVHeap() { return m_imguiSRVHeap; }
		//----Screen----//
		uint32 GetWidth() { return m_width; }
		uint32 GetHeight() { return m_height; }

		// Warning Off
	private:
		// debuging messeage OFF
		void OffWarningRenderTargetClearValue();

	private:
		uint32 m_width = 0;
		uint32 m_height = 0;
		HWND m_hwnd;

		// Device
		ComPtr<ID3D12Device5> m_device = nullptr;
		ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
		ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
		UINT m_swapChainFlags;

		uint32 m_frameIndex = 0;

		// RTV
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_rtvHeap;
		std::shared_ptr<Ideal::D3D12DynamicDescriptorHeap> m_imguiSRVHeap;

		uint32 m_rtvDescriptorSize = 0;
		//ComPtr<ID3D12Resource> m_renderTargets[SWAP_CHAIN_FRAME_COUNT];
		std::shared_ptr<Ideal::D3D12Texture> m_renderTargets[SWAP_CHAIN_FRAME_COUNT];


		// DSV
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
		ComPtr<ID3D12Resource> m_depthStencil = nullptr;

		// Command
		ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
		ComPtr<ID3D12CommandAllocator> m_commandAllocators[MAX_PENDING_FRAME_COUNT] = {};
		ComPtr<ID3D12GraphicsCommandList> m_commandLists[MAX_PENDING_FRAME_COUNT] = {};
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_descriptorHeaps[MAX_PENDING_FRAME_COUNT] = {};
		std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> m_cbAllocator[MAX_PENDING_FRAME_COUNT] = {};
		uint64 m_lastFenceValues[MAX_PENDING_FRAME_COUNT] = {};
		uint64 m_currentContextIndex = 0;

		// Fence
		ComPtr<ID3D12Fence> m_fence = nullptr;
		uint64 m_fenceValue = 0;
		HANDLE m_fenceEvent = NULL;

		// RenderScene
		//std::weak_ptr<Ideal::IdealRenderScene> m_currentRenderScene;
		std::shared_ptr<Ideal::IdealRenderScene> m_currentRenderScene;

	private:
		// D3D12 Data Manager
		std::shared_ptr<Ideal::D3D12DescriptorHeap> m_descriptorHeap = nullptr;

		std::shared_ptr<Ideal::DeferredDeleteManager> m_deferredDeleteManager = nullptr;

	private:
		float m_aspectRatio = 0.f;

	private:
		//Ideal::D3D12Viewport m_viewport;
		std::shared_ptr<Ideal::D3D12Viewport> m_viewport = nullptr;

	private:
		// Resource Manager
		std::shared_ptr<Ideal::ResourceManager> m_resourceManager = nullptr;

		// Main Camera
		std::shared_ptr<Ideal::IdealCamera> m_mainCamera = nullptr;

	private:
		std::wstring m_assetPath;
		std::wstring m_modelPath;
		std::wstring m_texturePath;

		// EDITOR 
	private:
		void InitImGui();

		Ideal::D3D12DescriptorHandle m_imguiSRVHandle;
		bool m_isEditor;

		// IMGUI
		void DrawImGuiMainCamera();
		void SetImGuiCameraRenderTarget();
		void ResizeImGuiMainCameraWindow(uint32 Width, uint32 Height);
		void CreateEditorRTV(uint32 Width, uint32 Height);
		// Editor RTV // 2024.06.01
		std::shared_ptr<Ideal::D3D12DynamicDescriptorHeap> m_editorRTVHeap;
		std::shared_ptr<Ideal::D3D12Texture> m_editorTexture;
	};
}