#pragma once
#include "../Utils/SimpleMath.h"
#include "ILight.h"
using namespace DirectX::SimpleMath;

namespace Ideal
{
	class IDirectionalLight : public ILight
	{
	public:
		IDirectionalLight()
			: ILight(ELightType::Directional)
		{}
		virtual ~IDirectionalLight() {}

	public:
		virtual void SetAmbientColor(const Color& LightColor) abstract;
		virtual void SetDiffuseColor(const Color& LightColor) abstract;
		virtual void SetDirection(const Vector3& Direction) abstract;
		virtual void SetIntensity(const float& Intensity) abstract;
	};
}