#include "GraphicsEngine/Resource/UI/IdealText.h"
#include "GraphicsEngine/D2D/D2DTextManager.h"

#include "GraphicsEngine/Resource/UI/IdealSprite.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"

Ideal::IdealText::IdealText(std::shared_ptr<Ideal::D2DTextManager> TextManager, std::shared_ptr<Ideal::FontHandle> FontHandle)
{
	m_textManager = TextManager;
	m_fontHandle = FontHandle;
}

Ideal::IdealText::~IdealText()
{
	if (textImage)
	{
		delete[] textImage;
	}
}

void Ideal::IdealText::ChangeText(const std::wstring& Text, const DirectX::SimpleMath::Color& Color)
{
	m_text = Text;
	m_isChanged = true;

	auto desc = m_textSprite->GetTexture().lock()->GetResource()->GetDesc();
	uint32 width = desc.Width;
	uint32 height = desc.Height;

	if (textImage)
	{
		delete[] textImage;
	}
	textImage = new BYTE[width * height * 4];

	m_textManager.lock()->WriteTextToBitmap(textImage, width, height, width * 4, m_fontHandle, Text.c_str(), Color);

}

std::shared_ptr<Ideal::IdealSprite> Ideal::IdealText::GetSprite()
{
	return m_textSprite;
}

void Ideal::IdealText::SetSprite(std::shared_ptr<Ideal::IdealSprite> Sprite)
{
	m_textSprite = Sprite;
	if (m_texture)
		m_textSprite->SetTexture(m_texture);
}

void Ideal::IdealText::SetTexture(std::shared_ptr<Ideal::D3D12Texture> Texture)
{
	m_texture = Texture;
	if(m_textSprite)
		m_textSprite->SetTexture(Texture);
}

void Ideal::IdealText::SetFontHandle(std::shared_ptr<Ideal::FontHandle> FontHandle)
{
	m_fontHandle = FontHandle;
}

void Ideal::IdealText::UpdateDynamicTextureWithImage(ComPtr<ID3D12Device> Device)
{
	if (!m_isChanged)
		return;
	m_isChanged = false;
	std::weak_ptr<Ideal::D3D12Texture> Texture = m_textSprite->GetTexture();

	D3D12_RESOURCE_DESC desc = Texture.lock()->GetResource()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
	uint32 rows = 0;
	uint64 rowSize = 0;
	uint64 TotalBytes = 0;
	Device->GetCopyableFootprints(&desc, 0, 1, 0, &Footprint, &rows, &rowSize, &TotalBytes);

	BYTE* mappedPtr = nullptr;
	CD3DX12_RANGE writeRange(0, 0);
	HRESULT hr = Texture.lock()->GetUploadBuffer()->Map(0, &writeRange, reinterpret_cast<void**>(&mappedPtr));
	Check(hr);

	BYTE* pSrc = textImage;
	BYTE* pDest = mappedPtr;
	for (uint32 y = 0; y < desc.Height; ++y)
	{
		memcpy(pDest, pSrc, desc.Width * 4);
		pSrc += (desc.Width * 4);
		pDest += Footprint.Footprint.RowPitch;
	}
	Texture.lock()->GetUploadBuffer()->Unmap(0, nullptr);
	Texture.lock()->SetUpdated();
}

void Ideal::IdealText::Resize(uint32 Width, uint32 Height)
{
	m_textSprite->ReSize(Width, Height);
}

void Ideal::IdealText::ResizeTexture(float Width, float Height)
{

}

