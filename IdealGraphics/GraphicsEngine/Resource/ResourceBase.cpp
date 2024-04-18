#include "GraphicsEngine/Resource/ResourceBase.h"

using namespace Ideal;

ResourceBase::ResourceBase()
{

}

ResourceBase::~ResourceBase()
{

}

void ResourceBase::SetName(const std::string& Name)
{
	m_name = Name;
}

const std::string& ResourceBase::GetName()
{
	return m_name;
}
