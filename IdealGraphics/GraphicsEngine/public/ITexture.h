#pragma once

namespace Ideal
{
	class ITexture
	{
	public:
		ITexture() {}
		virtual ~ITexture() {}

	public:
		// 사용법 : ImGui::Image((ImTextureID)GetImageID(), ImVec2) -> ImVec2는 이미지 사이즈
		virtual unsigned long long GetImageID() abstract;

	public:
		virtual unsigned int GetWidth() abstract;
		virtual unsigned int GetHeight() abstract;

	private:
	};
}