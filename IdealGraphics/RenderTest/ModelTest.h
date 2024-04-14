#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ResourceTest.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"
#include "Misc/ASEParser.h"

#include "MeshTest.h"
class ModelTest
{
public:
	ModelTest(std::shared_ptr<TestGraphics> graphics);
	virtual ~ModelTest();

	void LoadASEData(std::string path);
	void LoadFBXData(std::string path);

	void Render();
public:

private:
	std::shared_ptr<TestGraphics> m_graphics;
	std::shared_ptr<ASEData> m_aseData = nullptr; 

private:
	std::vector<MeshTest> m_meshes;

};

