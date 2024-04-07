#include "Engine/Engine2.h"
#include "RenderTest/ModelTest.h"

Engine2::Engine2(HWND hwnd, uint32 width, uint32 height)
{
	m_hwnd = hwnd;

	m_width = width;
	m_height = height;
}

Engine2::~Engine2()
{

}

bool Engine2::Init()
{
	m_graphics = std::make_shared<TestGraphics>(m_hwnd, m_width, m_height);
	m_graphics->Init();

	std::shared_ptr<ModelTest> model = std::make_shared<ModelTest>(m_graphics);
	model->LoadFBXData("Assets/JaneD/JaneD.fbx");
	//model->LoadASEData("RenderTest/box.ASE");
	//model->LoadASEData("RenderTest/genji_max.ASE");
	m_models.push_back(model);
	
	return true;
}

void Engine2::Tick()
{
	m_graphics->Tick();
}

void Engine2::Render()
{
	m_graphics->BeginRender();

	for (auto m : m_models)
	{
		m->Render();
	}

	m_graphics->EndRender();
}
