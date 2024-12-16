#include "IdealSpotLight.h"

Ideal::IdealSpotLight::IdealSpotLight()
{
	m_spotLight.Color = Color(1.f, 1.f, 1.f, 1.f);
	m_spotLight.Direction = Vector3(-1.f, 1.f, 0.f);
	m_spotLight.Position = Vector3(0.f, 0.f, 0.f);
	m_spotLight.SpotAngle = 37.7f;
	m_spotLight.Range = 4.7f;
	m_spotLight.Intensity = 3.4f;
	m_spotLight.Softness = 1.f;
}

Ideal::IdealSpotLight::~IdealSpotLight()
{

}

void Ideal::IdealSpotLight::SetLightColor(const Color& LightColor)
{
	m_spotLight.Color = LightColor;
}

void Ideal::IdealSpotLight::SetDirection(const Vector3& Direction)
{
	m_spotLight.Direction = Direction;
}

void Ideal::IdealSpotLight::SetPosition(const Vector3& Position)
{
	m_spotLight.Position = Position;
}

void Ideal::IdealSpotLight::SetSpotAngle(const float& SpotAngle)
{
	m_spotLight.SpotAngle = SpotAngle;
}

void Ideal::IdealSpotLight::SetRange(const float& Range)
{
	m_spotLight.Range = Range;
}

void Ideal::IdealSpotLight::SetIntensity(const float& Intensity)
{
	m_spotLight.Intensity = Intensity;
}

void Ideal::IdealSpotLight::SetSoftness(const float& Softness)
{
	m_spotLight.Softness = Softness;
}
