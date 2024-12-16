#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/VertexInfo.h"

namespace Ideal
{
	template <typename>
	class IdealMesh;
	class IdealRenderer;
	class D3D12PipelineStateObject;
	class D3D12Texture;
}

struct ID3D12RootSignature;

namespace Ideal
{
	class IdealScreenQuad
	{
		static const uint32 GBUFFER_SRV_NUM = 4;
	public:
		IdealScreenQuad();
		virtual ~IdealScreenQuad();

	public:
		void Init(std::shared_ptr<IdealRenderer> Renderer);
		void Draw(std::shared_ptr<IdealRenderer> Renderer, std::vector<std::shared_ptr<Ideal::D3D12Texture>>& GBufferTextures, std::shared_ptr<Ideal::D3D12Texture> DSTexture);
		void Resize(std::shared_ptr<IdealRenderer> Renderer);

	private:
		std::shared_ptr<Ideal::IdealMesh<QuadVertex>> m_quadMesh;
		std::vector<QuadVertex> m_vertices;
		std::vector<uint32> m_indices;

		std::shared_ptr<Ideal::D3D12PipelineStateObject> m_screenQuadPSO;
		ComPtr<ID3D12RootSignature> m_screenQuadRootSignature;
	};
}