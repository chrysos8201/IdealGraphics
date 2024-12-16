#pragma once
#include "../ThruthGameEngine/Headers.h"

using std::string;
using std::wstring;


/// <summary>
/// 유틸리티 기능을 제공하는 클래스
/// </summary>
class Utiles
{
public:
	// string -> wstring
	// wstring -> string
	static wstring ToWString(string value);
	static string ToString(wstring value);

	// 문자열 교체 
	static void Replace(string& str, string comp, string rep);
	static void Replace(wstring& str, wstring comp, wstring rep);

	// 특정 문자열로 시작하는지 여부 검사
	static bool StartsWith(string str, string comp);
	static bool StartsWith(wstring str, wstring comp);
};

