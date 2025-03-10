#include "GraphicsEngine/D2D/D2DTextManager.h"

//#include <d3d11on12.h>
//#include <d2d1_3.h>

Ideal::D2DTextManager::D2DTextManager()
{

}

Ideal::D2DTextManager::~D2DTextManager()
{

}

void Ideal::D2DTextManager::Init(ComPtr<ID3D12Device> D3D12Device, ComPtr<ID3D12CommandQueue> D3D12CommandQueue)
{
	CreateD2D(D3D12Device, D3D12CommandQueue);
	CreateDWrite(1024, 256, 96.f);	// Temp
}

void Ideal::D2DTextManager::WriteTextToBitmap(BYTE* pDestImage, uint32 DestWidth, uint32 DestHeight, uint32 DestPitch, std::shared_ptr<Ideal::FontHandle> FontHandle, const std::wstring& Text, const DirectX::SimpleMath::Color& Color)
{
	//uint32 textWidth = 0;
	//uint32 textHeight = 0;
	//
	//bool bResult = CreateBitmapFromText(&textWidth, &textHeight, FontHandle->TextFormat, Text, Color);
	//if (bResult)
	//{
	//	if (textWidth > (uint32)DestWidth)
	//	{
	//		textWidth = (uint32)DestWidth;
	//	}
	//	if (textHeight > (uint32)DestHeight)
	//	{
	//		textHeight = (uint32)DestHeight;
	//	}
	//
	//	D2D1_MAPPED_RECT mappedRect;
	//	Check(m_d2dTargetBitmapRead->Map(D2D1_MAP_OPTIONS_READ, &mappedRect));
	//
	//	BYTE* pDest = pDestImage;
	//	char* pSrc = (char*)mappedRect.bits;
	//		
	//	for (uint32 y = 0; y < textHeight; ++y)
	//	{
	//		memcpy(pDest, pSrc, textWidth * 4);
	//		pDest += DestPitch;
	//		pSrc += mappedRect.pitch;
	//	}
	//	m_d2dTargetBitmapRead->Unmap();
	//}
}

//bool Ideal::D2DTextManager::CreateBitmapFromText(uint32* OutWidth, uint32* OutHeight, ComPtr<IDWriteTextFormat> TextFormat, const std::wstring& Text, const DirectX::SimpleMath::Color& Color)
//{
//	bool result = false;
//
//	//D2D1_SIZE_F maxSize = m_d2dDeviceContext->GetSize();
//	//maxSize.width = (float)m_d2dBitmapWidth;
//	//maxSize.height = (float)m_d2dBitmapHeight;
//	//
//	//ComPtr<IDWriteTextLayout> textLayout = nullptr;
//	//if (m_dwriteFactory && TextFormat)
//	//{
//	//	Check(m_dwriteFactory->CreateTextLayout(Text.c_str(), Text.size(), TextFormat.Get(), maxSize.width, maxSize.height, textLayout.GetAddressOf()));
//	//}
//	//DWRITE_TEXT_METRICS metrics = {};
//	//if (textLayout)
//	//{
//	//	ComPtr<ID2D1SolidColorBrush> ColorBrush;
//	//	D2D1::ColorF d2d1Color(Color.R(), Color.G(), Color.B(), Color.A());
//	//	Check(m_d2dDeviceContext->CreateSolidColorBrush(d2d1Color, ColorBrush.GetAddressOf()));
//	//
//	//	textLayout->GetMetrics(&metrics);
//	//	// 타겟 설정
//	//	m_d2dDeviceContext->SetTarget(m_d2dTargetBitmap.Get());
//	//	// 안티앨리어싱모드 설정
//	//	m_d2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
//	//	// 텍스트 렌더링
//	//	m_d2dDeviceContext->BeginDraw();
//	//	//m_d2dDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));	// 검은색으로 클리어. 여기서 바꾸면 투명도 될 것 같음
//	//	m_d2dDeviceContext->Clear(D2D1::ColorF(0,0,0,0));	// 검은색으로 클리어. 여기서 바꾸면 투명도 될 것 같음
//	//	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
//	//	m_d2dDeviceContext->DrawTextLayout(D2D1::Point2F(0.f, 0.f), textLayout.Get(), ColorBrush.Get());
//	//	// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
//	//	// is lost. It will be handled during the next call to Present.
//	//	Check(m_d2dDeviceContext->EndDraw(), L"Failed To End Draw D2D");
//	//
//	//	// 안티앨리어싱 모드 복구
//	//	m_d2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
//	//	m_d2dDeviceContext->SetTarget(nullptr);
//	//
//	//	// 레이아웃 오브젝트 필요없음 -> ComPtr로 자동 Release
//	//}
//	//uint32 width = (uint32)ceil(metrics.width); // 소수점 올림
//	//uint32 height = (uint32)ceil(metrics.height);
//	//
//	//D2D1_POINT_2U destPos = {};
//	//D2D1_RECT_U srcRect = { 0, 0, width, height };
//	//Check(m_d2dTargetBitmapRead->CopyFromBitmap(&destPos, m_d2dTargetBitmap.Get(), &srcRect));
//	//
//	//*OutWidth = width;
//	//*OutHeight = height;
//
//	result = true;
//
//	return result;
//}

void Ideal::D2DTextManager::CreateD2D(ComPtr<ID3D12Device> D3D12Device, ComPtr<ID3D12CommandQueue> D3D12CommandQueue)
{
// 	UINT	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
// 	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
// #ifdef _DEBUG
// 	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
// 	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
// #endif
// 
// 	ComPtr<ID3D11On12Device> d3d11On12Device;
// 	ComPtr<ID3D11Device> d3d11Device;
// 	ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
// 	ComPtr<ID2D1Factory3> d2dFactory;
// 	ComPtr<IDXGIDevice> dxgiDevice;
// 
// 	Check(D3D11On12CreateDevice(
// 		D3D12Device.Get(),
// 		d3d11DeviceFlags,
// 		nullptr,
// 		0,
// 		reinterpret_cast<IUnknown**>(D3D12CommandQueue.GetAddressOf()),
// 		1,
// 		0,
// 		&d3d11Device,
// 		&d3d11DeviceContext,
// 		nullptr
// 	), L"Failed to create D3D11On12Device");
// 	Check(d3d11Device->QueryInterface(IID_PPV_ARGS(&d3d11On12Device)), L"Failed to query interface D3D11On12Device");
// 
// 	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
// 
// 	Check(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)d2dFactory.GetAddressOf()));
// 	Check(d3d11On12Device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf())));
// 	Check(d2dFactory->CreateDevice(dxgiDevice.Get(), m_d2dDevice.GetAddressOf()));
// 	Check(m_d2dDevice->CreateDeviceContext(deviceOptions, m_d2dDeviceContext.GetAddressOf()));

}

void Ideal::D2DTextManager::CreateDWrite(uint32 TexWidth, uint32 TexHeight, float DPI)
{
// 	m_d2dBitmapWidth = TexWidth;
// 	m_d2dBitmapHeight = TexHeight;
// 
// 	D2D1_SIZE_U size;
// 	size.width = TexWidth;
// 	size.height = TexHeight;
// 
// 	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
// 		D2D1::BitmapProperties1(
// 			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
// 			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
// 			DPI,
// 			DPI
// 		);
// 	Check(m_d2dDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, m_d2dTargetBitmap.GetAddressOf()));
// 	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ;
// 	Check(m_d2dDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, m_d2dTargetBitmapRead.GetAddressOf()));
// 	Check(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_whiteBrush.GetAddressOf()));
// 	Check(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), reinterpret_cast<IUnknown**>(m_dwriteFactory.GetAddressOf())));
}

std::shared_ptr<Ideal::FontHandle> Ideal::D2DTextManager::CreateTextObject(const wchar_t* FontFamilyName, float FontSize)
{
// 	ComPtr<IDWriteTextFormat> textFormat = nullptr;
// 	ComPtr<IDWriteFontCollection1> fontCollection = nullptr;
// 
// 	if (m_dwriteFactory)
// 	{
// 		Check(m_dwriteFactory->CreateTextFormat(
// 			FontFamilyName,
// 			fontCollection.Get(),
// 			//DWRITE_FONT_WEIGHT_REGULAR,
// 			DWRITE_FONT_WEIGHT_NORMAL,
// 			DWRITE_FONT_STYLE_NORMAL,
// 			DWRITE_FONT_STRETCH_NORMAL,
// 			FontSize,
// 			L"ko-kr",
// 			textFormat.GetAddressOf()
// 		));
// 	}
// 	std::shared_ptr<Ideal::FontHandle> newFontHandle = std::make_shared<Ideal::FontHandle>();
// 	wcscpy_s(newFontHandle->FontFamilyName, FontFamilyName);
// 	newFontHandle->FontSize = FontSize;
// 
// 	if (textFormat)
// 	{
// 		Check(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
// 		Check(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
// 	}
// 	newFontHandle->TextFormat = textFormat;
// 
// 	return newFontHandle;
	return nullptr;
}
