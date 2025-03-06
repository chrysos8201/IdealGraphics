#include "D3D12Util.h"
#include <d3d12.h>
#include <format>

//#define D3DERR(x) case x: ErrorCodeText = std::format("{}", #x); break;

static std::string GetD3D12DeviceHungErrorString(HRESULT ErrorCode)
{
	std::string ErrorCodeText;

	//switch (ErrorCode)
	//{
	//	D3DERR(DXGI_ERROR_DEVICE_HUNG)
	//		D3DERR(DXGI_ERROR_DEVICE_REMOVED)
	//		D3DERR(DXGI_ERROR_DEVICE_RESET)
	//		D3DERR(DXGI_ERROR_DRIVER_INTERNAL_ERROR)
	//		D3DERR(DXGI_ERROR_INVALID_CALL)
	//	default:
	//		ErrorCodeText = std::format("Error: {}", (int32)ErrorCode);
	//}

	return ErrorCodeText;
}

static std::string GetD3D12ErrorString(HRESULT ErrorCode, ID3D12Device* Device)
{
	std::string ErrorCodeText;
	//switch (ErrorCode)
	//{
	//	D3DERR(S_OK);
	//	D3DERR(D3D11_ERROR_FILE_NOT_FOUND)
	//		D3DERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
	//		D3DERR(E_FAIL)
	//		D3DERR(E_INVALIDARG)
	//		D3DERR(E_OUTOFMEMORY)
	//		D3DERR(DXGI_ERROR_INVALID_CALL)
	//		D3DERR(DXGI_ERROR_WAS_STILL_DRAWING)
	//		D3DERR(E_NOINTERFACE)
	//		D3DERR(DXGI_ERROR_DEVICE_REMOVED)
	//	default:
	//		ErrorCodeText = std::format("Error: {}", (int32)ErrorCode);
	//}
	//
	//if (ErrorCode == DXGI_ERROR_DEVICE_REMOVED && Device)
	//{
	//	HRESULT resultDeviceRemoved = Device->GetDeviceRemovedReason();
	//	ErrorCodeText += std::format("with Reason: ") + GetD3D12DeviceHungErrorString(resultDeviceRemoved);
	//}

	return ErrorCodeText;
}

extern void VerifyD3D12Result(HRESULT D3DResult, const ANSICHAR* Code, const ANSICHAR* FileName, uint32 Line, ComPtr<ID3D12Device> Device)
{
	if (FAILED(D3DResult))
	{
		const std::string& ErrorString = GetD3D12ErrorString(D3DResult, Device.Get());
		std::string message;// = std::format("{} failed \n at {}:{} \n with error {}\n", Code, FileName, Line, ErrorString);
		MyMessageBox(message.c_str());
	}
}