#pragma once

#include "Definitions.h"
#include "Types.h"

#include <windows.h>


#include <memory>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>

#include <algorithm>
#include <wrl.h>



#include "Misc/ThirdParty/Utils/Graph.h"
#include "Misc/ThirdParty/Thirdparty.h"
#include "../Utils/SimpleMath.h"
#include <cassert>
//#include <wrl.h>
//using namespace Microsoft::WRL;
using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

inline size_t AlignConstantBufferSize(size_t size)
{
	size_t aligned_size = (size + 255) & (~255);
	return aligned_size;
}


inline void Check(HRESULT hr)
{
	if (FAILED(hr))
	{
		MessageBox(NULL, L"어디선가", L"Check", MB_OK);
		assert(false);
	}
}

inline void Check(HRESULT hr, const wchar_t* Message)
{
	if (FAILED(hr))
	{
		if (Message == nullptr)
		{
			return;
		}
		MessageBox(NULL, Message, L"Check", MB_OK);
 		assert(false);
	}
}

inline void MyMessageBoxW(const wchar_t* Message)
{
	if (Message == nullptr)
	{
		return;
	}
	MessageBox(NULL, Message, L"Failed", MB_OK);
	assert(false);
}

inline void MyMessageBox(const char* Message)
{
	if (Message == nullptr)
	{
		return;
	}
	MessageBoxA(NULL, Message, "Failed", MB_OK);
	assert(false);
}

inline void MyDebugConsoleMessage(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


//auto Start = std::chrono::high_resolution_clock::now();
//auto End = std::chrono::high_resolution_clock::now();
//std::chrono::duration<double> duration = End - Start;
//std::string msg = "Load Times : " + std::to_string(duration.count()) + " seconds\n";
//MyDebugConsoleMessage(msg.c_str());