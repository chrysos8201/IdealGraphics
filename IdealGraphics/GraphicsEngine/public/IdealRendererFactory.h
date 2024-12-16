#pragma once

#include <memory>
#include <string>

// 사실 d3d12밖에 없음
enum class EGraphicsInterfaceType
{
	D3D12,
	D3D12_EDITOR,
	D3D12_RAYTRACING,
	D3D12_RAYTRACING_EDITOR,
};

namespace Ideal
{
	class IdealRenderer;
}

// AssetPath : 기존 데이터
// ModelPath : 커스텀 포맷으로 변경 하고 둘 파일 위치
// TexturePath: 커스텀 포맷 & Texture 복사 하고 둘 파일 위치
std::shared_ptr<Ideal::IdealRenderer> CreateRenderer(
	const EGraphicsInterfaceType& APIType,
	void* Hwnd,
	unsigned int Width,
	unsigned int Height,
	const std::wstring& AssetPath,
	const std::wstring& ModelPath,
	const std::wstring& TexturePath
	);
