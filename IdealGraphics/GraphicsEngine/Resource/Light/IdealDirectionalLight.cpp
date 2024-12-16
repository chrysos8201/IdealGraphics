#include "IdealDirectionalLight.h"

Ideal::IdealDirectionalLight::IdealDirectionalLight()
{
	m_directionalLight.AmbientColor = Color(0.3f, 0.3f, 0.3f, 1.f);
	m_directionalLight.DiffuseColor = Color(1.f, 1.f, 1.f, 1.f);
	Vector3 dir = Vector3(1.f, 1.f, 1.f);
	dir.Normalize();
	m_directionalLight.Direction = dir;
	m_directionalLight.Intensity = 1.f;
}

Ideal::IdealDirectionalLight::~IdealDirectionalLight()
{

}

void Ideal::IdealDirectionalLight::SetAmbientColor(const Color& LightColor)
{
	m_directionalLight.AmbientColor = LightColor;
}

void Ideal::IdealDirectionalLight::SetDiffuseColor(const Color& LightColor)
{
	m_directionalLight.DiffuseColor = LightColor;
}

void Ideal::IdealDirectionalLight::SetDirection(const Vector3& Direction)
{
	m_directionalLight.Direction = Direction;
}

void Ideal::IdealDirectionalLight::SetIntensity(const float& Intensity)
{
	m_directionalLight.Intensity = Intensity;
}
