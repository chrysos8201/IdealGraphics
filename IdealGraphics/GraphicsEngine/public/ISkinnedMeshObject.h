#pragma once
#include "IMeshObject.h"
#include "../Utils/SimpleMath.h"
#include <string>
#include <memory>

namespace Ideal
{
	class IAnimation;
}
namespace Ideal
{
	class ISkinnedMeshObject : public IMeshObject
	{
	public:
		ISkinnedMeshObject() {}
		virtual ~ISkinnedMeshObject() {}

	public:
		virtual void AddAnimation(const std::string& AnimationName, std::shared_ptr<Ideal::IAnimation> Animation) abstract;
		virtual void SetAnimation(const std::string& AnimationName, bool WhenCurrentAnimationFinished = true) abstract;
		virtual unsigned int GetCurrentAnimationIndex() abstract;
		virtual unsigned int GetCurrentAnimationMaxFrame() abstract;	// 현재 애니메이션의 마지막 프레임
		virtual void SetAnimationSpeed(float Speed) abstract;
		virtual void AnimationDeltaTime(const float& DeltaTime) abstract;

		virtual void SetPlayAnimation(bool Play) abstract;
	};
}