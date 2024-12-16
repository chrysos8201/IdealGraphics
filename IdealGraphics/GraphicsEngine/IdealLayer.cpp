#include "IdealLayer.h"

Ideal::IdealLayer::IdealLayer()
{
	m_Layer = 0;
}

Ideal::IdealLayer::~IdealLayer()
{

}

void Ideal::IdealLayer::AddLayer(uint32 LayerNum)
{
	m_Layer |= 1 << LayerNum;
}

void Ideal::IdealLayer::DeleteLayer(uint32 LayerNum)
{
	m_Layer &= ~LayerNum;
}

void Ideal::IdealLayer::ChangeLayer(uint32 LayerNum)
{
	m_Layer = 1 << LayerNum;
}

void Ideal::IdealLayer::ChangeBitMask(uint32 LayerMask)
{
	m_Layer = LayerMask;
}
