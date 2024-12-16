#pragma once
#include "IText.h"
#include "Core/Core.h"
#include <string>
#include <memory>
#include "GraphicsEngine/Resource/UI/IdealSprite.h"

namespace Ideal
{
	struct FontHandle;
	class IdealSprite;
	class D2DTextManager;
	class D3D12Texture;
	class ITexture;
}
struct ID3D12Device;

namespace Ideal
{
	class IdealText : public IText
	{
	public:
		IdealText(std::shared_ptr<Ideal::D2DTextManager> TextManager, std::shared_ptr<Ideal::FontHandle> FontHandle);
		~IdealText();

	public:
		virtual void ChangeText(const std::wstring& Text, const DirectX::SimpleMath::Color& Color = DirectX::SimpleMath::Color(1, 1, 1, 1)) override;
		std::shared_ptr<Ideal::IdealSprite> GetSprite();
		void SetSprite(std::shared_ptr<Ideal::IdealSprite> Sprite);
		// 자기만의 텍스쳐를 가진다.
		void SetTexture(std::shared_ptr<Ideal::D3D12Texture> Texture);
		void SetFontHandle(std::shared_ptr<Ideal::FontHandle> FontHandle);
		void UpdateDynamicTextureWithImage(ComPtr<ID3D12Device> Device);
		void Resize(uint32 Width, uint32 Height);
	public:
		// Sprite Interface
		virtual void SetActive(bool b) override { m_textSprite->SetActive(b); }
		virtual bool GetActive() override { return m_textSprite->GetActive(); }
		virtual float GetZ() override { return m_textSprite->GetZ(); }
		virtual float GetAlpha() override { return m_textSprite->GetAlpha(); }
		virtual void SetSampleRect(const SpriteRectArea& Rect) override { m_textSprite->SetSampleRect(Rect); };
		virtual void SetPosition(const DirectX::SimpleMath::Vector2& Position) override { m_textSprite->SetPosition(Position); };
		virtual void SetScale(const DirectX::SimpleMath::Vector2& Scale) override { m_textSprite->SetScale(Scale); };
		virtual void SetZ(float Z) override { m_textSprite->SetZ(Z); };
		virtual void SetAlpha(float Alpha) override { m_textSprite->SetAlpha(Alpha); };
		virtual void SetColor(const DirectX::SimpleMath::Color& Color) override { m_textSprite->SetColor(Color); };
		//virtual void SetTexture(std::weak_ptr<Ideal::ITexture> Texture) override { m_textSprite->SetTexture(Texture); };

		virtual void ResizeTexture(float Width, float Height) override;

		std::shared_ptr<Ideal::D3D12Texture> GetTexture() { return m_texture; }

	private:
		std::weak_ptr<Ideal::D2DTextManager> m_textManager;
		std::shared_ptr<Ideal::D3D12Texture> m_texture;
		std::shared_ptr<Ideal::FontHandle> m_fontHandle;
		std::shared_ptr<Ideal::IdealSprite> m_textSprite;
		std::wstring m_text;
		bool m_isChanged = false;
		BYTE* textImage = nullptr;
	};
}

