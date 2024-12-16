#include "Core/Core.h"
#include "IdealAnimation.h"
#include "GraphicsEngine/Resource/ModelAnimation.h"

Ideal::IdealAnimation::IdealAnimation()
{

}

Ideal::IdealAnimation::~IdealAnimation()
{

}

std::shared_ptr<Ideal::ModelKeyframe> Ideal::IdealAnimation::GetKeyframe(const std::string& name)
{
	auto findIt = keyframes.find(name);
	if (findIt == keyframes.end())
		return nullptr;
	return findIt->second;
}
