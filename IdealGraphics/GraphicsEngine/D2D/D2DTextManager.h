#pragma once
#include "Core/Core.h"
#include <d3d12.h>
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
//#include <dwrite_3.h>
//struct ID2D1Device2;
//struct ID2D1DeviceContext2;
//struct ID2D1Bitmap1;
//struct ID2D1SolidColorBrush;
struct ID3D12Device;
struct ID3D12CommandQueue;

namespace Ideal
{
	struct FontHandle
	{
		FontHandle()
		{
			//TextFormat = nullptr;
			FontSize = 0;
			memset(FontFamilyName, 0, sizeof(wchar_t) * 512);
		}
		//ComPtr<IDWriteTextFormat> TextFormat;
		float FontSize;
		wchar_t FontFamilyName[512];
	};

	class D2DTextManager
	{
	public:
		D2DTextManager();
		~D2DTextManager();

	public:
		void Init(ComPtr<ID3D12Device> D3D12Device, ComPtr<ID3D12CommandQueue> D3D12CommandQueue);

		void WriteTextToBitmap(BYTE* DestImage, uint32 DestWidth, uint32 DestHeight, uint32 DestPitch, std::shared_ptr<Ideal::FontHandle> FontHandle, const std::wstring& Text, const DirectX::SimpleMath::Color& Color);
		//bool CreateBitmapFromText(uint32* OutWidth, uint32* OutHeight, ComPtr<IDWriteTextFormat> TextFormat, const std::wstring& Text, const DirectX::SimpleMath::Color& Color);

		std::shared_ptr<Ideal::FontHandle> CreateTextObject(const wchar_t* FontFamilyName, float FontSize);
	private:
		void CreateD2D(ComPtr<ID3D12Device> D3D12Device, ComPtr<ID3D12CommandQueue> D3D12CommandQueue);
		void CreateDWrite(uint32 TexWidth, uint32 TexHeight, float DPI);

	private:
		// D2D
		//ComPtr<ID2D1Device2> m_d2dDevice;
		//ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
		//
		//ComPtr<ID2D1Bitmap1> m_d2dTargetBitmap;
		//ComPtr<ID2D1Bitmap1> m_d2dTargetBitmapRead;
		//
		//ComPtr<ID2D1SolidColorBrush> m_whiteBrush;
		//ComPtr<IDWriteFactory5> m_dwriteFactory;

		uint32 m_d2dBitmapWidth = 0;
		uint32 m_d2dBitmapHeight = 0;
	};
}
