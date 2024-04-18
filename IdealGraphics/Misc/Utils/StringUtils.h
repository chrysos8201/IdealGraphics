#pragma once
#include "Core/Core.h"

namespace StringUtils
{
	std::wstring ConvertStringToWString(const std::string& str);
	
	std::string ConvertWStringToString(const std::wstring& wstr);
	
}