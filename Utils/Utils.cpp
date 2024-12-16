#include "Utils.h"


/// <summary>
/// wstring 을 string 으로 변환
/// </summary>
std::string Utils::ToString(std::wstring _value)
{
	return std::string(_value.begin(), _value.end());
}

/// <summary>
/// string 을 wstring 으로 변환
/// </summary>
std::wstring Utils::ToWString(std::string _value)
{
	return std::wstring(_value.begin(), _value.end());
}

/// <summary>
/// 문자열 str 에서 comp 를 찾아서 rep 로 교체하는 함수
/// 문자열을 수정하고 변경한다.
/// </summary>
/// <param name="str">대상 문자열</param>
/// <param name="comp">교체할 부분 문자열</param>
/// <param name="rep">comp를 교체할 새 부분 문자열</param>
void Utils::Replace(std::string& _str, std::string _comp, std::string _rep)
{
	std::string temp = _str;

	size_t startPos = 0;

	// find 함수는 실패시 npos 를 반환
	while ((startPos = temp.find(_comp, startPos)) != std::string::npos)
	{
		temp.replace(startPos, _comp.length(), _rep);
		startPos += _rep.length();
	}

	_str = temp;
}

void Utils::Replace(std::wstring& _str, std::wstring _comp, std::wstring _rep)
{
	std::wstring temp = _str;

	size_t startPos = 0;
	while ((startPos = temp.find(_comp, startPos)) != std::wstring::npos)
	{
		temp.replace(startPos, _comp.length(), _rep);
		startPos += _rep.length();
	}

	_str = temp;
}

/// <summary>
/// 주어진 문자열이 특정 문자열로 시작하는지 여부를 확인하는 함수
/// </summary>
/// <param name="str">주어진 문자열</param>
/// <param name="comp">시작 지점 예상 문자열</param>
/// <returns></returns>
bool Utils::StartsWith(std::string _str, std::string _comp)
{
	std::wstring::size_type index = _str.find(_comp);
	if (index != std::wstring::npos && index == 0)
	{
		return true;
	}

	return false;
}

bool Utils::StartsWith(std::wstring _str, std::wstring _comp)
{
	std::wstring::size_type index = _str.find(_comp);
	if (index != std::wstring::npos && index == 0)
	{
		return true;
	}

	return false;
}