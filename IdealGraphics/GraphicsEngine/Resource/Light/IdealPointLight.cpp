                     #include "IdealPointLight.h"

Ideal::IdealPointLight::IdealPointLight()
{
	m_pointLight.Color = Color(1.f, 1.f, 1.f, 1.f);
	m_pointLight.Position = Vector3(0.f, 0.f, 0.f);
	m_pointLight.Range = 10.f;
	m_pointLight.Intensity = 1.f;
	SetShadowCasting(true);
	AddLayer(0);
}

Ideal::IdealPointLight::~IdealPointLight()
{

}

DirectX::SimpleMath::Color Ideal::IdealPointLight::GetLightColor()
{
	return m_pointLight.Color;
}

void Ideal::IdealPointLight::SetLightColor(const Color& LightColor)
{
	m_pointLight.Color = LightColor;
}

void Ideal::IdealPointLight::SetPosition(const Vector3& LightPosition)
{
	m_pointLight.Position = LightPosition;
}

void Ideal::IdealPointLight::SetRange(const float& Range)
{
	m_pointLight.Range = Range;
}

void Ideal::IdealPointLight::SetIntensity(const float& Intensity)
{
	m_pointLight.Intensity = Intensity;
}

void Ideal::IdealPointLight::SetShadowCasting(bool Active)
{
	m_pointLight.IsShadowCasting = (uint32)Active;
}

bool Ideal::IdealPointLight::GetIsShadowCasting()
{
	return (bool)m_pointLight.IsShadowCasting;
}

void Ideal::IdealPointLight::AddLayer(uint32 LayerNum)
{
	m_layer.AddLayer(LayerNum);
	m_pointLight.Layer = m_layer.GetLayer();
}

void Ideal::IdealPointLight::DeleteLayer(uint32 LayerNum)
{
	m_layer.DeleteLayer(LayerNum);
	m_pointLight.Layer = m_layer.GetLayer();
}

void Ideal::IdealPointLight::ChangeLayer(uint32 LayerNum)
{
	m_layer.ChangeLayer(LayerNum);
	m_pointLight.Layer = m_layer.GetLayer();
}

void Ideal::IdealPointLight::ChangeLayerBitMask(uint32 BitMask)
{
	m_layer.ChangeBitMask(BitMask);
	m_pointLight.Layer = m_layer.GetLayer();
}
