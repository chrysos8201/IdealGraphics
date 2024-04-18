#pragma once

#include <d3dcompiler.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <wrl.h>
//#include <d3dx12.h>
#include "ThirdParty/Common/d3dx12.h"

using namespace Microsoft::WRL;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "ThirdParty/Include/DirectXTK12/SimpleMath.h"
#pragma comment(lib, "ForDebug/DirectXTK12/DirectXTK12.lib")

using namespace DirectX::SimpleMath;

inline void Check(HRESULT hr)
{
	if (FAILED(hr))
	{
		assert(false);
	}
}