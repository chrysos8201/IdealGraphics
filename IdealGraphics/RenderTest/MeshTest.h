#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"
#include "RenderTest/D3D12ResourceTest.h"
#include "Misc/ASEParser.h"
#include "RenderTest/VertexInfo.h"
#include "Engine/TestGraphics.h"

class MeshTest
{
public:
	MeshTest();
	~MeshTest();

	void Create(std::shared_ptr<TestGraphics> graphics);
	void SetVertices(std::vector<Vertex>& vertices);
	void SetIndices(std::vector<uint32>& indices);

	void Render();
private:
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::vector<std::shared_ptr<ConstantBuffer>> m_constantBuffer;
	std::shared_ptr<RootSignature> m_rootSignature;
	std::shared_ptr<PipelineState> m_pipelineState;

public:
	// Vertices
	// Indices
	std::vector<Vertex> m_vertices;
	std::vector<uint32> m_indices;
	std::wstring m_diffuseMap;

	std::shared_ptr<TestGraphics> m_graphics;

};

