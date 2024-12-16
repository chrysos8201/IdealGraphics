#pragma once
#include "IMesh.h"
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12Renderer.h"
// Resource Head
#include "GraphicsEngine/Resource/IdealBone.h"
#include "GraphicsEngine/Resource/IdealMaterial.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
//#include "GraphicsEngine/D3D12/D3D12Renderer.h"
#include "GraphicsEngine/D3D12/ResourceManager.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/Raytracing/RayTracingFlagManger.h"
#include <d3d12.h>
#include <d3dx12.h>
namespace Ideal
{
	class IdealMaterial;
	class IMesh;
}

namespace Ideal
{
	template <typename TVertexType>
	class IdealMesh : public ResourceBase, public IMesh
	{
		//friend class IdealStaticMeshObject;

	public:
		IdealMesh()
			: m_vertexBuffer(nullptr),
			m_indexBuffer(nullptr),
			m_boneIndex(0)
		{}
		virtual ~IdealMesh() {};

	public:
		virtual std::string GetName() override { return m_name; }
		virtual void SetMaterialObject(std::shared_ptr<Ideal::IMaterial> Material) override
		{
			Ideal::Singleton::RayTracingFlagManger::GetInstance().SetMaterialChanged();
			m_material = std::static_pointer_cast<Ideal::IdealMaterial>(Material);
		}
		virtual std::weak_ptr<Ideal::IMaterial> GetMaterialObject() override { return m_material; }
		virtual std::string GetFBXMaterialName() override { return m_materialName; }
	public:
		void Create(std::shared_ptr<Ideal::ResourceManager> ResourceManager, bool Particle = false)
		{
			//-------------VB-------------//
			m_vertexBuffer = std::make_shared<Ideal::D3D12VertexBuffer>();
			ResourceManager->CreateVertexBuffer<TVertexType>(m_vertexBuffer, m_vertices);

			//-------------IB-------------//
			if (m_indices.size())
			{
				m_indexBuffer = std::make_shared<Ideal::D3D12IndexBuffer>();
				ResourceManager->CreateIndexBuffer(m_indexBuffer, m_indices);
			}
			//------------Clear-----------//
			//// 2024.09.05 Particle일 때는 Vertex를 지우지 않는다.
			//// -> Custom Data로 채워넣기 위함
			//if (!Particle)
			//{
			//	m_vertices.clear();
			//}
			m_vertices.clear();
			m_indices.clear();

			//----------Material----------//
			//if (m_material)
			//{
			//	m_material->Create(ResourceManager);
			//}
		}


		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return m_vertexBuffer->GetView(); }
		std::shared_ptr<D3D12VertexBuffer> GetVertexBuffer() { return m_vertexBuffer; }
		void SetVertexBuffer(std::shared_ptr<Ideal::D3D12VertexBuffer> VB) { m_vertexBuffer = VB; }
		D3D12_INDEX_BUFFER_VIEW	GetIndexBufferView() { return m_indexBuffer->GetView(); }
		std::shared_ptr<D3D12IndexBuffer> GetIndexBuffer() { return m_indexBuffer; }
		void SetIndexBuffer(std::shared_ptr<Ideal::D3D12IndexBuffer> IB) { m_indexBuffer = IB; }
		const uint32& GetElementCount() const { return m_indexBuffer->GetElementCount(); }

		void AddVertices(const std::vector<TVertexType>& vertices)
		{
			m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
		}
		std::vector<TVertexType>& GetVerticesRef()
		{
			return m_vertices;
		}
		void AddIndices(const std::vector<uint32>& indices)
		{
			m_indices.insert(m_indices.end(), indices.begin(), indices.end());
		}

		const std::string& GetMaterialName() { return m_materialName; }
		void SetMaterialName(const std::string& MaterialName) { m_materialName = MaterialName; }
		std::weak_ptr<Ideal::IdealMaterial> GetMaterial() { return m_material; }
		void SetMaterial(std::shared_ptr<IdealMaterial> Material) { m_material = Material; }
		void SetBoneIndex(const int32& Index) { m_boneIndex = Index; }
		void SetLocalTM(const Matrix& matrix) { m_localTM = matrix; }
		Matrix GetLocalTM() { return m_localTM; }

		template <typename T>
		void TransferMaterialInfo(std::shared_ptr<IdealMesh<T>> other)
		{
			m_material = other->m_material;
		}

		void SetMinMaxBound(Vector3 MinBound, Vector3 MaxBound) { m_minBound = MinBound; m_maxBound = MaxBound; }
		Vector3 GetMinBound() { return m_minBound; }
		Vector3 GetMaxBound() { return m_maxBound; }
	private:
		std::shared_ptr<D3D12VertexBuffer>	m_vertexBuffer;
		std::shared_ptr<D3D12IndexBuffer>	m_indexBuffer;
		
		std::vector<TVertexType>			m_vertices;
		std::vector<uint32>					m_indices;

		std::weak_ptr<Ideal::IdealMaterial> m_material;
		std::string m_materialName;

		std::vector<Ideal::IdealBone> m_bones;
		int32 m_boneIndex;

		Matrix m_localTM;

		Vector3 m_minBound;
		Vector3 m_maxBound;

	public:
		Ideal::D3D12ConstantBuffer m_constantBuffer;
	};
}