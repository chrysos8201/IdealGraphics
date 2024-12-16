#pragma once
#include "GraphicsEngine/public/ISpotLight.h"
#include "GraphicsEngine/ConstantBufferInfo.h"

namespace Ideal
{
	class IdealSpotLight : public ISpotLight
	{
	public:
		IdealSpotLight();
		virtual ~IdealSpotLight();

	public:
		virtual Color GetLightColor() override { return m_spotLight.Color; }
		virtual void SetLightColor(const Color& LightColor) override;
		
		virtual Vector3 GetDirection() override { return m_spotLight.Direction; }
		virtual void SetDirection(const Vector3& Direction) override;

		virtual Vector3 GetPosition() override { return m_spotLight.Position; }
		virtual void SetPosition(const Vector3& Position) override;

		virtual float GetSpotAngle()override { return m_spotLight.SpotAngle; }
		virtual void SetSpotAngle(const float& SpotAngle) override;

		virtual float GetRange() override { return m_spotLight.Range; }
		virtual void SetRange(const float& Range) override;

		virtual float GetIntensity() override { return m_spotLight.Intensity; }
		virtual void SetIntensity(const float& Intensity) override;

		virtual float GetSoftness() override { return m_spotLight.Softness; }
		virtual void SetSoftness(const float& Softness) override;

	public:
		SpotLight const& GetSpotLightDesc() { return m_spotLight; }

	private:
		SpotLight m_spotLight;
	};
}