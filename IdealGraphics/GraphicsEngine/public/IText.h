#pragma once
#include <string>
#include "ISprite.h"
namespace Ideal
{
	class IText
	{
	public:
		IText() {}
		virtual ~IText() {}

	public:
		virtual void ChangeText(const std::wstring& text, const DirectX::SimpleMath::Color& Color = DirectX::SimpleMath::Color(1,1,1,1)) abstract;

	public:
		// Sprite Change
		virtual void SetActive(bool) abstract;
		virtual bool GetActive() abstract;
		virtual float GetZ() abstract;
		virtual float GetAlpha() abstract;
		virtual void SetSampleRect(const SpriteRectArea&) abstract;
		virtual void SetPosition(const DirectX::SimpleMath::Vector2&) abstract;
		virtual void SetScale(const DirectX::SimpleMath::Vector2&) abstract;
		virtual void SetZ(float) abstract;
		virtual void SetAlpha(float) abstract;
		virtual void SetColor(const DirectX::SimpleMath::Color&) abstract;
		//virtual void SetTexture(std::weak_ptr<Ideal::ITexture>) abstract;

		virtual void ResizeTexture(float Width, float Height) abstract;

	};
}