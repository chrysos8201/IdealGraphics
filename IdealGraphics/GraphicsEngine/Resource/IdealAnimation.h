#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/public/IAnimation.h"
#include "GraphicsEngine/ConstantBufferInfo.h"

namespace Ideal
{
	struct ModelKeyframe;
}

namespace Ideal
{
	struct AnimTransform
	{
		using TransformArrayType = std::array<Matrix, MAX_BONE_TRANSFORMS>;
		std::array<TransformArrayType, MAX_MODEL_KEYFRAMES> transforms;

	};
	class IdealAnimation : public IAnimation
	{
	public:
		IdealAnimation();
		virtual ~IdealAnimation();

		virtual uint32 GetFrameCount() override { return frameCount; }
		virtual float GetFrameRate() override { return frameRate; }

	public:
		std::shared_ptr<ModelKeyframe> GetKeyframe(const std::string& name);

		std::string name;
		float duration = 0.f;
		float frameRate = 0.f;
		uint32 frameCount = 0;
		int32 numBones = 0;
		std::unordered_map<std::string, std::shared_ptr<ModelKeyframe>> keyframes;
		std::shared_ptr<Ideal::AnimTransform> m_animTransform;
	};
}