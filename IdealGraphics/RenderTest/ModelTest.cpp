#include "RenderTest/ModelTest.h"
#include "Misc/AssimpLoader.h"
ModelTest::ModelTest(std::shared_ptr<TestGraphics> graphics)
	: m_graphics(graphics)
{

}

ModelTest::~ModelTest()
{

}

void ModelTest::LoadASEData(std::string path)
{
	// ���� § �ڵ�� ����ȭ�� �ȵ�����.

	m_aseData = ASEParser::LoadASE3(path);

	// �޽��� �����
	uint32 meshNum = m_aseData->geomObjects.size();
	for (uint32 i = 0; i < meshNum; i++)
	{
		// �޽��� �����
		MeshTest mesh;
		std::vector<Vertex> v;
		
		// �޽��ȿ� �ִ� ���ؽ� ������ �����ϰ�
		uint32 meshVertexNum = m_aseData->geomObjects[i]->vertices.size();
		for (uint32 j = 0; j < meshVertexNum; j++)
		{
			ASEData::Vertex aseVertex = m_aseData->geomObjects[i]->vertices[j];
			v.push_back({ aseVertex.pos,aseVertex.normal, aseVertex.uv });
		}

		// �޽� ������ �ٽ� �־��ش�.
		mesh.SetVertices(v);

		// �̹����� �ε��� ����
		mesh.SetIndices(m_aseData->geomObjects[i]->indices);

		mesh.Create(m_graphics);

		m_meshes.push_back(mesh);
	}
}

void ModelTest::LoadFBXData(std::string path)
{
	ImportSettings2 importSetting =
	{
		path,
		m_meshes,
		false,
		true
	};
	AssimpLoader loader;
	if (!loader.Load(importSetting))
	{
		assert(false);
		return;
	}
	for (uint32 i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i].Create(m_graphics);
	}
}

void ModelTest::Render()
{
	for (auto& mesh : m_meshes)
	{
		mesh.Render();
	}
}