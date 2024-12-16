#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/VertexInfo.h"

struct BasicVertex;

namespace Ideal
{
	template <typename>
	class IdealMesh;
	class IdealMaterial;
	class IdealRenderer;
	class IdealBone;

	class ResourceManager;
}

namespace Ideal
{
	class IdealStaticMesh : public ResourceBase
	{
	public:
		IdealStaticMesh();
		virtual ~IdealStaticMesh();

	public:
		void Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer, const Matrix& WorldTM);
		void DebugDraw(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList);
	public:
		void AddMesh(std::shared_ptr<Ideal::IdealMesh<BasicVertex>> Mesh);
		void AddMaterial(std::shared_ptr<Ideal::IdealMaterial> Material);
		void AddBone(std::shared_ptr<Ideal::IdealBone> Bone) { m_bones.push_back(Bone); }

		void FinalCreate(std::shared_ptr<Ideal::ResourceManager> ResourceManager);

		Matrix GetLocalTM();

	public:
		std::vector<std::shared_ptr<Ideal::IdealMesh<BasicVertex>>>& GetMeshes() { return m_meshes; }

	private:
		std::vector<std::shared_ptr<Ideal::IdealMesh<BasicVertex>>> m_meshes;
		std::vector<std::shared_ptr<Ideal::IdealBone>> m_bones;
	};
}