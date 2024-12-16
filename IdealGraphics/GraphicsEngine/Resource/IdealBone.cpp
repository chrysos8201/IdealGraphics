#include "GraphicsEngine/Resource/IdealBone.h"

Ideal::IdealBone::IdealBone()
	: m_boneName(),
	m_boneIndex(-1),
	m_parent(-1),
	m_transform(Matrix::Identity)
{

}
