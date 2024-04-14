#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12Resource.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"
#include "RenderTest/VertexInfo.h"

class IdealRenderer;

namespace Ideal
{
	class Mesh
	{
	public:
		Mesh();
		virtual ~Mesh();

	public:
		void Create(std::shared_ptr<IdealRenderer> Renderer);
		void Render(ID3D12GraphicsCommandList* CommandList);

	private:
		std::shared_ptr<IdealRenderer> m_renderer;

		D3D12VertexBuffer m_vertexBuffer;
		D3D12IndexBuffer m_indexBuffer;

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32> m_indices;
	};
}