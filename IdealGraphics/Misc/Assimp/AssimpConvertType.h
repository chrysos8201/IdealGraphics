#pragma once
#include "Core/Core.h"

#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

namespace AssimpConvert
{
	struct Mesh
	{
		std::string name;
		int32 boneIndex;	//���� ��� �ε��� ��
		std::string materialName;	// �� mesh�� �׸��� �ʿ��� material�� �̸��� ������ �ְڴ�.

		std::vector<BasicVertex> vertices;	// ������ ���� �ʿ䰡 ����
		std::vector<uint32> indices;	// ��������

		std::wstring diffuseMap;
	};

	struct Bone
	{
		std::string name;
		int32 index = -1;
		int32 parent = -1;
		Matrix transform;
	};

	struct Model
	{

	};
	
	struct Material
	{
		std::string name;
		Color ambient;
		Color specular;
		Color diffuse;
		Color emissive;

		std::string diffuseTextureFile;
		std::string specularTextureFile;
		std::string emissiveTextureFile;
		std::string normalTextureFile;
	};
}