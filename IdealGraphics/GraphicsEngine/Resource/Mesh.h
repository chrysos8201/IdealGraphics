#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/VertexInfo.h"

class IdealRenderer;
class AssimpLoader;

namespace Ideal
{
	class Model;
	class Material;
}

namespace Ideal
{
	class Mesh : public ResourceBase
	{
		friend class AssimpLoader;
		friend class Ideal::Model;

	public:
		Mesh();
		virtual ~Mesh();

	public:
		void Create(std::shared_ptr<IdealRenderer> Renderer);

		// Temp
		void Tick();

		void Render(ID3D12GraphicsCommandList* CommandList);

	private:
		void InitRootSignature();
		void InitPipelineState();

	private:
		std::shared_ptr<IdealRenderer> m_renderer;

		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12RootSignature> m_rootSignature;

		D3D12VertexBuffer m_vertexBuffer;
		D3D12IndexBuffer m_indexBuffer;
		D3D12ConstantBuffer m_constantBuffer;

		Transform m_transform;
		Transform* m_cbDataBegin;

	public:
		void AddVertices(const std::vector<BasicVertex>& vertices);
		void AddIndices(const std::vector<uint32>& indices);

	private:
		std::shared_ptr<Ideal::Material> m_material;

		int32 m_boneIndex;	//뼈일 경우 인덱스 값
		std::string m_materialName;	// 이 mesh를 그릴때 필요한 material의 이름을 가지고 있겠다.

		std::vector<BasicVertex> m_vertices;	// 나중에 가지고 있을 필요가 없다
		std::vector<uint32> m_indices;	// 마찬가지

		std::wstring m_diffuseMap;
	};
}