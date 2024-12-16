#include "Test.h"
#include <memory>
#include <vector>
#include "../IdealGraphics/Misc/Utils/FileUtils.h"

using namespace DirectX::SimpleMath;

void ReadVertexPosition(const std::wstring& path)
{
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(path, FileMode::Read);

	// 저장할 배열
	std::vector<Vector3> pos;

	unsigned int meshNum = file->Read<unsigned int>();

	for (int i = 0; i < meshNum; i++)
	{
		unsigned int verticesNum = file->Read<unsigned int>();
		for (int j = 0; j < verticesNum; j++)
		{
			Vector3 p;
			p.x = file->Read<float>();
			p.y = file->Read<float>();
			p.z = file->Read<float>();
			pos.push_back(p);
		}
	}

	int a = 3;
}
