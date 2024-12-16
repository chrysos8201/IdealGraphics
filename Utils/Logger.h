#pragma once
#include "windows.h"
#ifndef _DEBUG
#define DEBUG_PRINT
#else
#include <debugapi.h>
// static void _PrintDebugString(const WCHAR* format, ...) 
// {
// 	WCHAR buf[1024];
// 	va_list vaList;
// 	va_start(vaList, format);
// 	_vsnwprintf_s(buf, sizeof(buf), format, vaList);
// 	va_end(vaList);
// 	OutputDebugStringW(buf);
// }
static void _PrintDebugString(const char* format, ...) 
{
	char buf[2048];
	va_list vaList;
	va_start(vaList, format);
	_vsnprintf_s(buf, sizeof(buf), format, vaList);
	va_end(vaList);
	OutputDebugStringA(buf);
}
#define DEBUG_PRINT _PrintDebugString
#endif