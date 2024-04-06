#include "RenderTest/MeshTest.h"

MeshTest::MeshTest()
{

}

MeshTest::~MeshTest()
{

}

void MeshTest::Create(std::shared_ptr<TestGraphics> graphics)
{
	m_graphics = graphics;

#pragma region Vertex
	uint32 vertexSize = sizeof(Vertex) * m_vertices.size();
	uint32 vertexStride = sizeof(Vertex);

	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertexSize, vertexStride, m_vertices.data());
	if (!m_vertexBuffer->IsValid())
	{
		assert(false);
		return;
	}
#pragma endregion

#pragma region Index
	uint32 indexSize = sizeof(uint32) * m_indices.size();

	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, indexSize, m_indices.data());
	if (!m_indexBuffer->IsValid())
	{
		assert(false);
		return;
	}

#pragma endregion

#pragma region CS
	m_constantBuffer = std::vector<std::shared_ptr<ConstantBuffer>>(TestGraphics::FRAME_BUFFER_COUNT);

	Vector3 eyePos(0.f, 0.f, 5.f);
	Vector3 targetPos = Vector3::Zero;
	Vector3 upward(0.f, 1.f, 0.f);

	float fov = DirectX::XMConvertToRadians(37.5f);
	float aspect = static_cast<float>(1280) / static_cast<float>(960);

	for (uint32 i = 0; i < TestGraphics::FRAME_BUFFER_COUNT; i++)
	{
		m_constantBuffer[i] = std::make_shared<ConstantBuffer>(graphics, sizeof(Transform));
		if (!m_constantBuffer[i]->IsValid())
		{
			assert(false);
			return;
		}

		auto ptr = m_constantBuffer[i]->GetPtr<Transform>();
		ptr->World = Matrix::Identity;
		ptr->View = Matrix::CreateLookAt(eyePos, targetPos, upward);
		ptr->Proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 0.3f, 1000.f);
	}

#pragma endregion

#pragma region Root Signature
	m_rootSignature = std::make_shared<RootSignature>(graphics);
	if (!m_rootSignature->IsValid())
	{
		assert(false);
		return;
	}
#pragma endregion

#pragma region PipelineState
	m_pipelineState = std::make_shared<PipelineState>(graphics);
	m_pipelineState->SetInputLayout(Vertex::InputLayout);
	m_pipelineState->SetRootSignature(m_rootSignature->Get());
	m_pipelineState->SetVS(L"../x64/Debug/BoxVS.cso");
	m_pipelineState->SetPS(L"../x64/Debug/BoxPS.cso");
	m_pipelineState->Create();

	if (!m_pipelineState->IsValid())
	{
		assert(false);
		return;
	}
#pragma endregion
}

void MeshTest::SetVertices(std::vector<Vertex>& vertices)
{
	m_vertices = vertices;
}

void MeshTest::SetIndices(std::vector<uint32>& indices)
{
	m_indices = indices;
}

void MeshTest::Render()
{
	auto currentIndex = m_graphics->CurrentBackBufferIndex();
	auto commandList = m_graphics->GetCommandList();
	auto vbView = m_vertexBuffer->View();
	auto ibView = m_indexBuffer->View();

	commandList->SetGraphicsRootSignature(m_rootSignature->Get());
	commandList->SetPipelineState(m_pipelineState->Get());
	commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vbView);
	commandList->IASetIndexBuffer(&ibView);
	commandList->DrawIndexedInstanced(m_indices.size(), 1, 0, 0, 0);
}

