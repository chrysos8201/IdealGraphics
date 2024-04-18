#pragma once
#include "Core/Core.h"

#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

namespace AssimpConvert
{
	struct Mesh
	{
		std::string name;
		int32 boneIndex;	//뼈일 경우 인덱스 값
		std::string materialName;	// 이 mesh를 그릴때 필요한 material의 이름을 가지고 있겠다.

		std::vector<BasicVertex> vertices;	// 가지고 있을 필요가 없다
		std::vector<uint32> indices;	// 마찬가지

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