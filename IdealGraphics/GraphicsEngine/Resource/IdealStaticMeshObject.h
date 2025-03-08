#pragma once
#include "GraphicsEngine/public/IMeshObject.h"
#include "Core/Core.h"
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/Resource/IdealStaticMesh.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/IdealLayer.h"

struct ID3D12GraphicsCommandList;

namespace Ideal
{
	class IdealRenderer;
	class IdealStaticMesh;
	class D3D12Renderer;

	class D3D12DescriptorHeap2;
	class DeferredDeleteManager;
	class D3D12DynamicConstantBufferAllocator;
	//--raytracing--//
	class RaytracingManager;
	class DXRBottomLevelAccelerationStructure;
	class BLASInstanceDesc;
}


namespace Ideal
{
	class IdealStaticMeshObject : public IMeshObject, public std::enable_shared_from_this<IdealStaticMeshObject>
	{
	public:
		IdealStaticMeshObject();
		virtual ~IdealStaticMeshObject();
	public:
		// Interface //
		virtual uint32 GetMeshesSize() override;
		virtual std::weak_ptr<Ideal::IMesh> GetMeshByIndex(uint32 index) override;
		virtual uint32 GetBonesSize() override;
		virtual std::weak_ptr<Ideal::IBone> GetBoneByIndex(uint32 index) override;

	public:
		void Init(ComPtr<ID3D12Device> Device);
		// 2024.05.07 Ver2
		void Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer);

		void DebugDraw(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap2> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager);

	public:
		virtual void SetTransformMatrix(const Matrix& Transform) override { m_transform = Transform; m_isDirty = true; }
		virtual void SetDrawObject(bool IsDraw) override { m_isDraw = IsDraw; };

		virtual Matrix GetLocalTransformMatrix(){ return m_staticMesh->GetLocalTM(); };

		virtual Ideal::EMeshType GetMeshType() const override { return EMeshType::Static; }

		virtual void SetStaticWhenRunTime(bool Static) override;
		bool GetIsStaticWhenRunTime();

		virtual void SetDebugMeshColor(DirectX::SimpleMath::Color& Color) override;
		const Matrix& GetTransformMatrix() const { return m_transform; }
		void SetStaticMesh(std::shared_ptr<Ideal::IdealStaticMesh> Mesh) { m_staticMesh = Mesh; }
		std::shared_ptr<Ideal::IdealStaticMesh> GetStaticMesh() { return m_staticMesh; }
	
	private:
		std::shared_ptr<IdealStaticMesh> m_staticMesh;
		Matrix m_transform;
		bool m_isDraw = true;
		CB_Color m_cbDebugColor;
		bool m_isStaticWhenRuntime = false;

		//------Raytracing Info------//
	public:
		virtual void AlphaClippingCheck() override;
	public:
		void UpdateBLASInstance(std::shared_ptr<Ideal::RaytracingManager> RaytracingManager);
		void SetBLAS(std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> InBLAS);
		void SetBLASInstanceIndex(uint32 InstanceID) { m_instanceIndex = InstanceID; }
		void SetBLASInstanceDesc(std::shared_ptr<Ideal::BLASInstanceDesc> InstanceDesc) { m_BLASInstanceDesc = InstanceDesc; }
		std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> GetBLAS();
		std::shared_ptr<Ideal::BLASInstanceDesc> GetBLASInstanceDesc() { return m_BLASInstanceDesc; }

	private:
		bool m_isDirty = false;
		uint32 m_instanceIndex = 0;
		std::shared_ptr<Ideal::BLASInstanceDesc> m_BLASInstanceDesc;
		std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> m_blas;


		//-----Layer-----//
	public:
		virtual void AddLayer(uint32 LayerNum) override;
		virtual void DeleteLayer(uint32 LayerNum) override;
		virtual void ChangeLayer(uint32 LayerNum) override;

		Ideal::IdealLayer& GetLayer() { return m_Layer; }
	private:
		Ideal::IdealLayer m_Layer;
	};
}