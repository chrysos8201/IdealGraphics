#pragma once
#include <string>

namespace Ideal
{
	class IRendererResource
	{
	public:
		IRendererResource() {}
		virtual ~IRendererResource() {}

	public:
		std::wstring GetName() const { return m_name; }
		void SetName(const std::wstring& Name) { m_name = Name; }

	private:
		std::wstring m_name;
	};
}