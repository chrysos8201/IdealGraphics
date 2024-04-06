#include "RenderTest/ModelTest.h"

ModelTest::ModelTest(std::shared_ptr<TestGraphics> graphics)
	: m_graphics(graphics)
{

}

ModelTest::~ModelTest()
{

}

void ModelTest::LoadASEData(std::string path)
{
	// 대충 짠 코드라 최적화가 안돼있음.

	m_aseData = ASEParser::LoadASE3(path);

	// 메쉬가 몇개인지
	uint32 meshNum = m_aseData->geomObjects.size();
	for (uint32 i = 0; i < meshNum; i++)
	{
		// 메쉬를 만들고
		std::shared_ptr<MeshTest> mesh = std::make_shared<MeshTest>();
		std::vector<Vertex> v;
		
		// 메쉬안에 있는 버텍스 정보를 복사하고
		uint32 meshVertexNum = m_aseData->geomObjects[i]->vertices.size();
		for (uint32 j = 0; j < meshVertexNum; j++)
		{
			ASEData::Vertex aseVertex = m_aseData->geomObjects[i]->vertices[j];
			v.push_back({ aseVertex.pos,aseVertex.normal, aseVertex.uv });
		}

		// 메쉬 정보를 다시 넣어준다.
		mesh->SetVertices(v);

		// 이번에는 인덱스 정보
		mesh->SetIndices(m_aseData->geomObjects[i]->indices);

		mesh->Create(m_graphics);

		m_meshes.push_back(mesh);
	}
}

void ModelTest::Render()
{
	for (auto& mesh : m_meshes)
	{
		mesh->Render();
	}
}
