#pragma once
#include "GraphicsEngine/public/IPointLight.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/IdealLayer.h"

namespace Ideal
{
	class IdealPointLight : public IPointLight
	{
	public:
		IdealPointLight();
		virtual ~IdealPointLight();

	public:
		virtual Color GetLightColor() override;
		virtual void SetLightColor(const Color& LightColor) override;

		virtual Vector3 GetPosition() override { return m_pointLight.Position; }
		virtual void SetPosition(const Vector3& LightPosition) override;

		virtual float GetRange() override { return m_pointLight.Range; }
		virtual void SetRange(const float& Range) override;

		virtual float GetIntensity() override { return m_pointLight.Intensity; }
		virtual void SetIntensity(const float& Intensity) override;

		virtual void SetShadowCasting(bool Active) override;
		virtual bool GetIsShadowCasting() override;

		virtual void AddLayer(uint32 LayerNum) override;
		virtual void DeleteLayer(uint32 LayerNum) override;
		virtual void ChangeLayer(uint32 LayerNum) override;
		virtual void ChangeLayerBitMask(uint32 BitMask) override;

	public:
		PointLight const& GetPointLightDesc() { return m_pointLight; }

	private:
		PointLight m_pointLight;
		Ideal::IdealLayer m_layer;
	};
}