#pragma once
#include "Core/Core.h"

namespace Ideal
{
	class ResourceBase
	{
	public:
		ResourceBase();
		virtual ~ResourceBase();

	public:
		void SetName(const std::string& Name);
		const std::string& GetName();

	protected:
		std::string m_name;
	};
}