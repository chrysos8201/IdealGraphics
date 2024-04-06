#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12Resource.h"
#include "RenderTest/D3D12ThirdParty.h"
#include "RenderTest/ASEParser.h"

#include "MeshTest.h"
class ModelTest
{
public:
	ModelTest(std::shared_ptr<TestGraphics> graphics);
	virtual ~ModelTest();

	void LoadASEData(std::string path);

	void Render();
public:

private:
	std::shared_ptr<TestGraphics> m_graphics;
	//ASEData* m_aseData = nullptr; 
	std::shared_ptr<ASEData> m_aseData = nullptr; 
	std::vector<std::shared_ptr<MeshTest>> m_meshes;

};

