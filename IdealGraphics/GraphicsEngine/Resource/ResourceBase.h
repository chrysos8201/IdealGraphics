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

		uint64 GetID();
		void SetID(uint64 ID);

		void AddRefCount() { m_refCount++; }
		void SubRefCount() { m_refCount--; }
		uint32 GetRefCount() { return m_refCount; }

	protected:
		std::string m_name;
		uint64 m_id;
		uint32 m_refCount = 0;
	};
}