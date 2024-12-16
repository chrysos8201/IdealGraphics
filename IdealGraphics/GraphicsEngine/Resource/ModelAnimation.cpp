#include "GraphicsEngine/Resource/ModelAnimation.h"

using namespace Ideal;

std::shared_ptr<Ideal::ModelKeyframe> Ideal::ModelAnimation::GetKeyframe(const std::string& name)
{
	auto findIt = keyframes.find(name);
	if (findIt == keyframes.end())
		return nullptr;
	return findIt->second;
}
