#pragma once
#include "Core/Core.h"

namespace Ideal
{
	class IdealLayer
	{
	public:
		IdealLayer();
		~IdealLayer();

		void AddLayer(uint32 LayerNum);
		void DeleteLayer(uint32 LayerNum);

		// 아예 Layer자체를 바꾼다.
		void ChangeLayer(uint32 LayerNum);
		void ChangeBitMask(uint32 LayerMask);

		uint32 GetLayer() { return m_Layer; }
	private:
		uint32 m_Layer;
	};
}
