	#pragma once
#include "Core/Core.h"
#include "IRenderScene.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include <DirectXColors.h>

namespace Ideal
{
	class IMeshObject;
	class ILight;

	class IdealRenderer;
	class IdealStaticMeshObject;
	class IdealSkinnedMeshObject;
	class IdealDirectionalLight;
	class IdealSpotLight;
	class IdealPointLight;
	class IdealScreenQuad;

	class DeferredDeleteManager;

	// D3D12
	class D3D12PipelineStateObject;
	class D3D12Shader;
	class D3D12Texture;
}
// CB
struct CB_Global;
struct CB_LightList;

// ID3D12
struct ID3D12RootSignature;

namespace Ideal
{
	class IdealRenderScene : public IRenderScene
	{

	public:
		IdealRenderScene();
		virtual ~IdealRenderScene();

	public:
		void Init(std::shared_ptr<IdealRenderer> Renderer, std::shared_ptr<Ideal::DeferredDeleteManager> InDeferredDeleteManager);
		void Draw(std::shared_ptr<IdealRenderer> Renderer);
		void Resize(std::shared_ptr<IdealRenderer> Renderer);

		void Free();

		// 2024.05.15 : buffer render
		void DrawGBuffer(std::shared_ptr<IdealRenderer> Renderer);
		void DrawScreen(std::shared_ptr<IdealRenderer> Renderer);
		void DrawScreenEditor(std::shared_ptr<IdealRenderer> Renderer);

	public:
		//----------IRenderScene Interface-----------//
		virtual void AddObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) override;
		virtual void AddDebugObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) override;
		virtual void AddLight(std::shared_ptr<Ideal::ILight> Light) override;

		virtual void DeleteObject(std::shared_ptr<Ideal::IMeshObject> MeshObject);
		virtual void DeleteLight(std::shared_ptr<Ideal::ILight> Light);
		virtual void DeleteDebugObject(std::shared_ptr<Ideal::IMeshObject> MeshObject);

	private:
		// Ver2 : 2024.05.07 : cb pool, descriptor pool을 사용하는 방식으로 바꾸겠다.
		void CreateStaticMeshPSO(ID3D12Device* Device);
		void CreateSkinnedMeshPSO(ID3D12Device* Device);

		void CreateDebugStaticMeshPSO(ID3D12Device* Device);

		// 2024.05.17 : ScreenQuad Data
		void AllocateFromDescriptorHeap(std::shared_ptr<IdealRenderer> Renderer);
		void BindDescriptorTable(std::shared_ptr<IdealRenderer> Renderer);

		// 2024.05.13 : global cb
		void CreateGlobalCB(std::shared_ptr<IdealRenderer> Renderer);
		void UpdateGlobalCBData(std::shared_ptr<IdealRenderer> Renderer);

		// 2024.05.17 : lightList cb
		void CreateLightCB(std::shared_ptr<IdealRenderer> Renderer);
		void UpdateLightCBData(std::shared_ptr<IdealRenderer> Renderer);

		// 2024.05.15 : GBuffer
		void CreateGBuffer(std::shared_ptr<IdealRenderer> Renderer);
		void TransitionGBufferToRTVandClear(std::shared_ptr<IdealRenderer> Renderer);
		void TransitionGBufferToSRV(std::shared_ptr<IdealRenderer> Renderer);

		// 2024.05.19 : DSV
		void CreateDSV(std::shared_ptr<IdealRenderer> Renderer);

		// 2024.05.15 : MainScreenQuad
		void InitScreenQuad(std::shared_ptr<IdealRenderer> Renderer);
		void CreateScreenQuadRootSignature(ID3D12Device* Device);
		void CreateScreenQuadPSO(ID3D12Device* Device);
		void InitScreenQuadEditor(std::shared_ptr<Ideal::IdealRenderer> Renderer);

	private:
		//std::vector<std::shared_ptr<Ideal::IdealStaticMeshObject>> m_staticMeshObjects;
		//std::vector<std::shared_ptr<Ideal::IdealSkinnedMeshObject>> m_skinnedMeshObjects;

		std::vector<std::shared_ptr<Ideal::IdealStaticMeshObject>> m_staticMeshObjects;
		std::vector<std::shared_ptr<Ideal::IdealSkinnedMeshObject>> m_skinnedMeshObjects;
		std::vector<std::shared_ptr<Ideal::IdealStaticMeshObject>> m_debugMeshObjects;

		std::vector<std::shared_ptr<Ideal::IdealDirectionalLight>> m_directionalLights;
		std::vector<std::shared_ptr<Ideal::IdealSpotLight>> m_spotLights;
		std::vector<std::shared_ptr<Ideal::IdealPointLight>> m_pointLights;

	private:
		std::shared_ptr<Ideal::D3D12PipelineStateObject> m_staticMeshPSO;
		std::shared_ptr<Ideal::D3D12PipelineStateObject> m_skinnedMeshPSO;
		std::shared_ptr<Ideal::D3D12PipelineStateObject> m_debugStaticMeshPSO;

		ComPtr<ID3D12RootSignature> m_staticMeshRootSignature;
		ComPtr<ID3D12RootSignature> m_skinnedMeshRootSignature;

	private:
		std::shared_ptr<CB_Global> m_cbGlobal;
		D3D12DescriptorHandle m_cbGlobalHandle;

		std::shared_ptr<CB_LightList> m_cbLightList;

		// GBufferRenderTarget
		static const uint32 m_gBufferNum = 4;
		std::vector<std::shared_ptr<Ideal::D3D12Texture>> m_gBuffers;
		Color m_gBufferClearColors[m_gBufferNum];

		// DSV
		std::shared_ptr<Ideal::D3D12Texture> m_depthBuffer = nullptr;

		// MainScreen Texture
		std::shared_ptr<Ideal::D3D12Texture> m_screenBuffer = nullptr;

		// ScreenQuad
		std::shared_ptr<Ideal::IdealScreenQuad> m_fullScreenQuad;
		std::shared_ptr<Ideal::D3D12PipelineStateObject> m_screenQuadPSO;
		ComPtr<ID3D12RootSignature> m_screenQuadRootSignature;
		uint32 m_width;
		uint32 m_height;


		// Manager
		std::shared_ptr<DeferredDeleteManager> m_deferredDeleteManager;
	};
}