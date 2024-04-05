#include "Engine/Engine.h"
#include <DirectXMath.h>
Engine::Engine(HWND hwnd, uint32 width, uint32 height)
{
	m_hwnd = hwnd;

	m_width = width;
	m_height = height;
}

Engine::~Engine()
{

}

bool Engine::Init()
{
	m_graphics = std::make_shared<TestGraphics>(m_hwnd, m_width, m_height);
	m_graphics->Init();

#pragma region 정점 정리
	Vertex vertices[3] = {};
	vertices[0].Pos= Vector3(-1.0f, -1.0f, 0.0f);
	vertices[0].Color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

	vertices[1].Pos= Vector3(1.0f, -1.0f, 0.0f);
	vertices[1].Color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].Pos= Vector3(0.0f, 1.0f, 0.0f);
	vertices[2].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

	auto vertexSize = sizeof(Vertex) * std::size(vertices);
	auto vertexStride = sizeof(Vertex);
	m_vertexBuffer = std::make_shared<VertexBuffer>(m_graphics, vertexSize, vertexStride, vertices);
	if (!m_vertexBuffer->IsValid())
	{
		assert(false);
		return false;
	}
#pragma  endregion

#pragma region 상수 버퍼
	m_constantBuffer = std::vector<std::shared_ptr<ConstantBuffer>>(TestGraphics::FRAME_BUFFER_COUNT);
	
	Vector3 eyePos(0.f, 0.f, 5.f);
	Vector3 targetPos = Vector3::Zero;
	Vector3 upward(0.f, 1.f, 0.f);

	auto fov = DirectX::XMConvertToRadians(37.5f);
	auto aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

	for (uint32 i = 0; i < TestGraphics::FRAME_BUFFER_COUNT; i++)
	{
		m_constantBuffer[i] = std::make_shared<ConstantBuffer>(m_graphics, sizeof(Transform));
		if (!m_constantBuffer[i]->IsValid())
		{
			assert(false);
			return false;
		}

		auto ptr = m_constantBuffer[i]->GetPtr<Transform>();
		ptr->World = Matrix::Identity;
		ptr->View = Matrix::CreateLookAt(eyePos, targetPos, upward);
		ptr->Proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 0.3f, 1000.f);
	}
#pragma endregion

#pragma region 루트 시그니쳐
	m_rootSignature = std::make_shared<RootSignature>(m_graphics);
	if (!m_rootSignature->IsValid())
	{
		assert(false);
		return false;
	}

#pragma endregion

#pragma region 파이프라인 스테이트
	m_pipelineState = std::make_shared<PipelineState>(m_graphics);
	m_pipelineState->SetInputLayout(Vertex::InputLayout);
	m_pipelineState->SetRootSignature(m_rootSignature->Get());
	//m_pipelineState->SetVS(L"Shaders/BoxVS.hlsl");
	m_pipelineState->SetVS(L"../x64/Debug/BoxVS.cso");
	//m_pipelineState->SetPS(L"Shaders/BoxPS.hlsl");
	m_pipelineState->SetPS(L"../x64/Debug/BoxPS.cso");
	m_pipelineState->Create();

	if (!m_pipelineState->IsValid())
	{
		return false;
	}

#pragma endregion
	return true;
}

void Engine::Tick()
{
	m_graphics->Tick();
}

void Engine::Render()
{
	m_graphics->BeginRender();

	{
		auto currentIndex = m_graphics->CurrentBackBufferIndex();
		auto commandList = m_graphics->GetCommandList();
		auto vbView = m_vertexBuffer->View();

		commandList->SetGraphicsRootSignature(m_rootSignature->Get());
		commandList->SetPipelineState(m_pipelineState->Get());
		commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer[currentIndex]->GetAddress());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);

		commandList->DrawInstanced(3,1,0,0);
	}

	m_graphics->EndRender();
}
