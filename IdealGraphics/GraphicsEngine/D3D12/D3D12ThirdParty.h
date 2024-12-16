#pragma once

#include <d3dcompiler.h>
//#include <d3dx12.h>
//#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include <d3dx12.h>
#include <dxcapi.h>

// d3d11, d2d
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>


#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include "../Utils/SimpleMath.h"

#ifdef _DEBUG
#pragma comment(lib, "DebugLib/DirectXTK12/DirectXTK12.lib")
#pragma comment(lib, "DebugLib/DirectXTex/DirectXTex.lib")
#else
#pragma comment(lib, "ReleaseLib/DirectXTK12/DirectXTK12.lib")
#pragma comment(lib, "ReleaseLib/DirectXTex/DirectXTex.lib")
#endif



using namespace DirectX::SimpleMath;
inline void GetErrorBlob(ID3DBlob* ErrorBlob)
{
	if (ErrorBlob != nullptr)
	{
		const char* errorMessage = (char*)ErrorBlob->GetBufferPointer();
		__debugbreak();
	}
}