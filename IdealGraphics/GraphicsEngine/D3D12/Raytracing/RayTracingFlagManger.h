#pragma once
#include "Core/Core.h"

namespace Ideal
{
	class IdealMaterial;
}

namespace Ideal
{
	namespace Singleton
	{
		class RayTracingFlagManger
		{
		public:
			static RayTracingFlagManger& GetInstance()
			{
				static RayTracingFlagManger instance;
				return instance;
			}

			RayTracingFlagManger(const RayTracingFlagManger&) = delete;
			RayTracingFlagManger operator=(const RayTracingFlagManger&) = delete;

		private:
			RayTracingFlagManger() {}
			~RayTracingFlagManger() {}

		public:
			void SetMaterialChanged() { m_isMaterialChanged = true; }
			bool GetMaterialChangedAndOffFlag()
			{
				bool ret = m_isMaterialChanged;
				m_isMaterialChanged = false;
				return ret;
			}

			void SetTextureChanged() { m_isTextureChanged = true; }
			bool GetTextureChangedAndOffFlag() 
			{
				bool ret = m_isTextureChanged;
				m_isTextureChanged = false;
				return ret;
			}
		private:
			bool m_isTextureChanged = false;
			bool m_isMaterialChanged = false;
		};
	}
}

