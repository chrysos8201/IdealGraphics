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

uint64 ResourceBase::GetID()
{
	return m_id;
}

void ResourceBase::SetID(uint64 ID)
{
	m_id = ID;
}
