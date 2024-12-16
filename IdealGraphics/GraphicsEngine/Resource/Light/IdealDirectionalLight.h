#pragma once
#include "GraphicsEngine/public/IDirectionalLight.h"
#include "GraphicsEngine/ConstantBufferInfo.h"

namespace Ideal
{
	class IdealDirectionalLight : public IDirectionalLight
	{
	public:
		IdealDirectionalLight();
		virtual ~IdealDirectionalLight();

	public:
		virtual void SetAmbientColor(const Color& LightColor) override;
		virtual void SetDiffuseColor(const Color& LightColor) override;
		virtual void SetDirection(const Vector3& Direction) override;
		virtual void SetIntensity(const float& Intensity) override;

	public:
		DirectionalLight const& GetDirectionalLightDesc() { return m_directionalLight; }
		//DirectionalLight GetDirectionalLightDesc() { return m_directionalLight; }

	public:
		/*DirectionalLight operator=(const DirectionalLight& rhs)
		{
			m_directionalLight.AmbientColor = rhs.AmbientColor;
			m_directionalLight.DiffuseColor = rhs.DiffuseColor;
			m_directionalLight.Direction = rhs.Direction;
			m_directionalLight.Intensity = rhs.Intensity;
			return m_directionalLight;
		}*/

	private:
		DirectionalLight m_directionalLight;
	};
}