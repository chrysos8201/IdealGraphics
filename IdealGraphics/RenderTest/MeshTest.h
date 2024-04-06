#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "RenderTest/D3D12Resource.h"
#include "RenderTest/ASEParser.h"
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

private:
	// Vertices
	// Indices
	std::vector<Vertex> m_vertices;
	std::vector<uint32> m_indices;

	std::shared_ptr<TestGraphics> m_graphics;
};

