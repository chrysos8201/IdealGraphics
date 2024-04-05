#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"

struct alignas(256) Transform	// ����� ���߾���?
{
	Matrix World;
	Matrix View;
	Matrix Proj;
};

struct Vertex
{
	Vector3 Pos;
	Vector4 Color;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int32 InputElementCount = 2;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};
