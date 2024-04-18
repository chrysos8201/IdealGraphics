#include "Misc/Utils/StringUtils.h"

std::wstring StringUtils::ConvertStringToWString(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string StringUtils::ConvertWStringToString(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}
