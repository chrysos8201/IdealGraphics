#pragma once
#include "Engine/IEngine.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "VertexInfo.h"
#include "Engine/TestGraphics.h"
#include "RenderTest/D3D12Resource.h"

class Engine : public IEngine
{
public:
	Engine(HWND hwnd, uint32 width, uint32 height);
	virtual ~Engine();

public:
	virtual bool Init() override;
	virtual void Tick() override;
	virtual void Render() override;

private:
	std::shared_ptr<TestGraphics> m_graphics;
private:
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	//std::shared_ptr<ConstantBuffer> m_constantBuffer[TestGraphics::FRAME_BUFFER_COUNT];
	std::vector<std::shared_ptr<ConstantBuffer>> m_constantBuffer;
	std::shared_ptr<RootSignature> m_rootSignature;
	std::shared_ptr<PipelineState> m_pipelineState;

	HWND m_hwnd;

	uint32 m_width;
	uint32 m_height;

private:
	float m_rotateY = 0.f;
};

