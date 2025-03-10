#pragma once
#include "GraphicsEngine/public/ISkinnedMeshObject.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/Resource/IdealBone.h"
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/Raytracing/DXRAccelerationStructure.h"
#include "GraphicsEngine/IdealLayer.h"
#include <d3d12.h>

namespace Ideal
{
	class IAnimation;

	class IdealBone;
	class IdealRenderer;
	class IdealAnimation;
	class IdealSkinnedMesh;
	class RaytracingManager;
	class DXRBottomLevelAccelerationStructure;
	class ResourceManager;
	class D3D12DynamicConstantBufferAllocator;
	class D3D12UAVBuffer;
	class D3D12DescriptorHeap2;
	class D3D12UnorderedAccessView;
	class BLASInstanceDesc;
}
struct AnimTransform
{
	using TransformArrayType = std::array<Matrix, MAX_BONE_TRANSFORMS>;
	std::array<TransformArrayType, MAX_MODEL_KEYFRAMES> transforms;
};

enum class EAnimationState
{
	// 현재 애니메이션
	CurrentAnimation,
	// 애니메이션 바로 바꾸기
	ChangeAnimation,
	// 애니메이션 끝나고 바꾸기
	ReserveAnimation
};

namespace Ideal
{
	class IdealSkinnedMeshObject : public ISkinnedMeshObject, public std::enable_shared_from_this<IdealSkinnedMeshObject>
	{
	public:
		IdealSkinnedMeshObject();
		virtual ~IdealSkinnedMeshObject();

	public:
		// Interface //
		virtual uint32 GetMeshesSize() override;
		virtual std::weak_ptr<Ideal::IMesh> GetMeshByIndex(uint32 index) override;
		virtual uint32 GetBonesSize() override;
		virtual std::weak_ptr<Ideal::IBone> GetBoneByIndex(uint32 index) override;

	public:
		void Init(std::shared_ptr<IdealRenderer> Renderer);
		void Draw(std::shared_ptr<Ideal::IdealRenderer> Renderer);

	public:
		virtual void SetTransformMatrix(const Matrix& Transform) override { m_transform = Transform; m_isDirty = true; }
		virtual void SetDrawObject(bool IsDraw) override { m_isDraw = IsDraw; };

		virtual DirectX::SimpleMath::Matrix GetLocalTransformMatrix() { return m_transform; };

		virtual Ideal::EMeshType GetMeshType() const override { return Ideal::EMeshType::Skinned; }
		virtual void SetDebugMeshColor(DirectX::SimpleMath::Color& Color) override {};

		const Matrix& GetTransformMatrix() const { return m_transform; }
		void SetSkinnedMesh(std::shared_ptr<Ideal::IdealSkinnedMesh> Mesh);
		
		// 작동 안함
		virtual void SetStaticWhenRunTime(bool Static) override;

	private:
		bool m_isDraw = true;

		std::shared_ptr<Ideal::IdealSkinnedMesh> m_skinnedMesh;
		std::vector<std::shared_ptr<Ideal::IdealBone>> m_bones;

		CB_Bone m_cbBoneData;
		CB_Transform m_cbTransformData;

		Matrix m_transform;

		//------Raytracing Info------//
	public:
		virtual void AlphaClippingCheck() override;
	public:
		void CreateUAVVertexBuffer(ComPtr<ID3D12Device5> Device, std::shared_ptr<Ideal::ResourceManager> ResourceManager);
		std::shared_ptr<Ideal::IdealSkinnedMesh> GetSkinnedMesh() { return m_skinnedMesh; }
		void UpdateBLASInstance(
			ComPtr<ID3D12Device5> Device,
			ComPtr<ID3D12GraphicsCommandList4> CommandList,
			std::shared_ptr<Ideal::ResourceManager> ResourceManager,
			std::shared_ptr<Ideal::D3D12DescriptorHeap2> DescriptorManager,
			uint32 CurrentContextIndex,
			std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool,
			std::shared_ptr<Ideal::RaytracingManager> RaytracingManager,
			std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager
		);
		void SetBLAS(std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> InBLAS);
		std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> GetBLAS();
		//std::vector<BLASGeometry> GetBLASGeometries(ComPtr<ID3D12Device> Device, std::shared_ptr<Ideal::D3D12DescriptorManager> DescriptorManager);
		void SetBLASInstanceIndex(uint32 InstanceIndex);
		void SetBLASInstanceDesc(std::shared_ptr<Ideal::BLASInstanceDesc> InstanceDesc) { m_BLASInstanceDesc = InstanceDesc; }
		std::shared_ptr<Ideal::BLASInstanceDesc> GetBLASInstanceDesc() { return m_BLASInstanceDesc; }

		std::shared_ptr<Ideal::D3D12UAVBuffer> GetUAV_VertexBuffer() { return m_uavBuffer; }
		std::shared_ptr<Ideal::D3D12UAVBuffer> GetUAV_VertexBufferByIndex(const uint32& index) { return m_vertexBuffers[index]; }

	private:
		uint32 m_instanceIndex = 0;
		std::shared_ptr<Ideal::BLASInstanceDesc> m_BLASInstanceDesc;
		bool m_isDirty = false;
		std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> m_BLAS;

		std::shared_ptr<Ideal::D3D12UAVBuffer> m_uavBuffer;
		std::shared_ptr<Ideal::D3D12UnorderedAccessView> m_uavView;
		
		// mesh가 여러개일 경우 
		std::vector<std::shared_ptr<Ideal::D3D12UAVBuffer>> m_vertexBuffers;
		std::vector<std::shared_ptr<Ideal::D3D12UnorderedAccessView>> m_vertexBufferUAVs;


		// Animation
	public:
		virtual void AddAnimation(const std::string& AnimationName, std::shared_ptr<Ideal::IAnimation> Animation) override;
		virtual void SetAnimation(const std::string& AnimationName, bool WhenCurrentAnimationFinished = true) override;
		virtual uint32 GetCurrentAnimationIndex() override { return m_currentFrame; };
		virtual uint32 GetCurrentAnimationMaxFrame() override;
		virtual void SetAnimationSpeed(float Speed) override { m_animSpeed = Speed; }
		virtual void AnimationDeltaTime(const float& DeltaTime) override;
		virtual void SetPlayAnimation(bool Play) override { m_playAnimation = Play; }

	private:
		void AnimationPlay();
		void AnimationInterpolate(
			std::shared_ptr<Ideal::IdealAnimation> BeforeAnimation,
			uint32 BeforeAnimationFrame,
			std::shared_ptr<Ideal::IdealAnimation> NextAnimation,
			uint32 NextAnimationFrame
		);

	private:
		/// Ver2
		std::map<std::string, std::shared_ptr<Ideal::IdealAnimation>> m_animations;
		std::map<std::string, std::shared_ptr<AnimTransform>> m_animTransforms;
		std::shared_ptr<Ideal::IdealAnimation> m_currentAnimation;
		std::shared_ptr<Ideal::IdealAnimation> m_nextAnimation;

		// 다음 애니메이션이 있을 경우 추가
		bool m_reservedAnimation = false;
		bool m_isAnimationFinished = false;

		bool m_playAnimation = true;

		float m_sumTime = 0.f;
		uint32 m_currentFrame = 0;
		uint32 m_nextFrame = 0;
		float m_animSpeed = 1.f;
		float m_ratio = 0.f;

		float m_deltaTime = 0.f;

		//std::shared_ptr<Ideal::IdealAnimation> m_cacheCurrentAnimation;
		uint32 m_cacheCurrentAnimationFrame = 0;
		//std::shared_ptr<Ideal::IdealAnimation> m_cacheNextAnimation;


		EAnimationState m_animationState = EAnimationState::CurrentAnimation;

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