#include "GraphicsEngine/Mesh.h"

#include "GraphicsEngine/IdealRenderer.h"

Ideal::Mesh::Mesh()
{

}

Ideal::Mesh::~Mesh()
{

}

void Ideal::Mesh::Create(std::shared_ptr<IdealRenderer> Renderer)
{
	//--------------Init---------------//
	m_renderer = Renderer;


	//--------------VB---------------//
	{
		const uint32 vertexBufferSize = m_vertices.size() * sizeof(Vertex);

		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(Renderer->GetDevice().Get(), vertexBufferSize);
		{
			void* mappedUploadHeap = uploadBuffer.Map();
			//std::copy(m_vertices.begin(), m_vertices.end(), mappedUploadHeap);
			uploadBuffer.UnMap();
		}
		const uint32 elementSize = sizeof(Vertex);
		const uint32 elementCount = m_vertices.size();

		m_vertexBuffer.Create(
			m_renderer->GetDevice().Get(),
			m_renderer->GetCommandList().Get(),
			elementSize,
			elementCount,
			uploadBuffer
		);

		// Wait
		m_renderer->ExecuteCommandList();
	}

	//--------------IB---------------//
	{
		const uint32 indexBufferSize = m_indices.size() * sizeof(uint32);

		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(Renderer->GetDevice().Get(), indexBufferSize);
		{
			void* mappedUploadHeap = uploadBuffer.Map();
			//std::copy(m_indices.begin(), m_indices.end(), mappedUploadHeap);
			uploadBuffer.UnMap();
		}
		const uint32 elementSize = sizeof(uint32);
		const uint32 elementCount = m_indices.size();

		m_indexBuffer.Create(
			m_renderer->GetDevice().Get(),
			m_renderer->GetCommandList().Get(),
			elementSize,
			elementCount,
			uploadBuffer
		);

		// Wait
		m_renderer->ExecuteCommandList();
	}
}

void Ideal::Mesh::Render(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_vertexBuffer.GetView();
	CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = m_indexBuffer.GetView();
	CommandList->IASetIndexBuffer(&indexBufferView);

	CommandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
}
