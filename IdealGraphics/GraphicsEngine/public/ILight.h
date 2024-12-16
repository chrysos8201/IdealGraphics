#pragma once
#include "IdealRendererDefinitions.h"

namespace Ideal
{
	class ILight
	{
	public:
		ILight(ELightType LightType) 
			: m_lightType(LightType)
		{}
		virtual ~ILight() {}

		ELightType GetLightType() { return m_lightType; }

	private:
		ELightType m_lightType;
	};
}