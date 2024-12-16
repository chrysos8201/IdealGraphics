#pragma once
#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include <d3d12.h>
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

struct VertexTest2
{
	Vector3 Position;
	Vector2 UV;
};

struct PositionNormalVertex
{
	Vector3 Position;
	Vector3 Normal;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const DXGI_FORMAT VertexFormat = VERTEX_FORMAT;
	static const int32 InputElementCount = 2;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct PositionNormalUVVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const DXGI_FORMAT VertexFormat = VERTEX_FORMAT;
	static const int32 InputElementCount = 3;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct GeometryVertex
{
	Vector3 Position;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const DXGI_FORMAT VertexFormat = VERTEX_FORMAT;
	static const int32 InputElementCount = 1;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct BasicVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;
	Vector3 Tangent;
	Vector4 Color;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 5;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct SkinnedVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;
	Vector3 Tangent;
	Vector4 BlendIndices;
	Vector4 BlendWeights;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 6;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct QuadVertex
{
	Vector3 Position;
	Vector2 UV;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 2;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct SimpleVertex
{
	Vector4 Position;
	Vector2 TexCoord;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 2;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

//struct ParticleVertex
//{
//	Vector3 Position;
//	Vector3 Normal;
//	Vector4 UV0;
//	Vector4 UV1;
//	Vector4 UV2;
//	Vector4 Color;
//
//	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
//
//public:
//	static const int32 InputElementCount = 6;
//	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
//};

struct ParticleMeshVertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 3;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};


struct ParticleVertex
{
	Vector4 Color;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

public:
	static const int32 InputElementCount = 1;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};