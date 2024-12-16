#include "Utiles.h"


/// <summary>
/// wstring 을 string 으로 변환
/// </summary>
std::string Utiles::ToString(wstring value)
{
	return string(value.begin(), value.end());
}

/// <summary>
/// string 을 wstring 으로 변환
/// </summary>
std::wstring Utiles::ToWString(string value)
{
	return wstring(value.begin(), value.end());
}

/// <summary>
/// 문자열 str 에서 comp 를 찾아서 rep 로 교체하는 함수
/// 문자열을 수정하고 변경한다.
/// </summary>
/// <param name="str">대상 문자열</param>
/// <param name="comp">교체할 부분 문자열</param>
/// <param name="rep">comp를 교체할 새 부분 문자열</param>
void Utiles::Replace(string& str, string comp, string rep)
{
	string temp = str;

	size_t startPos = 0;

	// find 함수는 실패시 npos 를 반환
	while ((startPos = temp.find(comp, startPos)) != string::npos)
	{
		temp.replace(startPos, comp.length(), rep);
		startPos += rep.length();
	}

	str = temp;
}

void Utiles::Replace(wstring& str, wstring comp, wstring rep)
{
	wstring temp = str;

	size_t startPos = 0;
	while ((startPos = temp.find(comp, startPos)) != wstring::npos)
	{
		temp.replace(startPos, comp.length(), rep);
		startPos += rep.length();
	}

	str = temp;
}

/// <summary>
/// 주어진 문자열이 특정 문자열로 시작하는지 여부를 확인하는 함수
/// </summary>
/// <param name="str">주어진 문자열</param>
/// <param name="comp">시작 지점 예상 문자열</param>
/// <returns></returns>
bool Utiles::StartsWith(string str, string comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}

bool Utiles::StartsWith(wstring str, wstring comp)
{
	wstring::size_type index = str.find(comp);
	if (index != wstring::npos && index == 0)
		return true;

	return false;
}