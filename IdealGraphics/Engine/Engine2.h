#pragma once
#include "Engine/IEngine.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "RenderTest/VertexInfo.h"
#include "Engine/TestGraphics.h"
#include "RenderTest/D3D12Resource.h"

class ModelTest;

class Engine2 : public IEngine
{
public:
	Engine2(HWND hwnd, uint32 width, uint32 height);
	virtual ~Engine2();

public:
	virtual bool Init() override;
	virtual void Tick() override;
	virtual void Render() override;

private:
	std::vector<std::shared_ptr<ModelTest>> m_models;

private:
	std::shared_ptr<TestGraphics> m_graphics;
private:
	HWND m_hwnd;

	uint32 m_width;
	uint32 m_height;

private:
	float m_rotateY = 0.f;
};

