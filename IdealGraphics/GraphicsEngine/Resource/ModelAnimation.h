#pragma once
#include "Core/Core.h"

namespace Ideal
{
	struct ModelKeyFrameData
	{
		float time = 0.f;
		Vector3 scale;
		Quaternion rotation;
		Vector3 translation;
	};

	struct ModelKeyframe
	{
		std::string boneName;
		std::vector<ModelKeyFrameData> transforms;
	};

	struct ModelAnimation
	{
		std::shared_ptr<ModelKeyframe> GetKeyframe(const std::string& name);

		std::string name;
		float duration = 0.f;
		float frameRate = 0.f;
		uint32 frameCount = 0;
		std::unordered_map<std::string, std::shared_ptr<ModelKeyframe>> keyframes;
	};
}