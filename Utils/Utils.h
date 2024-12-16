#pragma once
#include <string>

#pragma warning (disable : 4244)

/// <summary>
/// 유틸리티 기능을 제공하는 클래스
/// </summary>
class Utils
{
public:
	// string -> wstring
	// wstring -> string
	static std::wstring ToWString(std::string _value);
	static std::string ToString(std::wstring _value);

	// 문자열 교체 
	static void Replace(std::string& _str, std::string _comp, std::string _rep);
	static void Replace(std::wstring& _str, std::wstring _comp, std::wstring _rep);

	// 특정 문자열로 시작하는지 여부 검사
	static bool StartsWith(std::string _str, std::string _comp);
	static bool StartsWith(std::wstring _str, std::wstring _comp);
};

