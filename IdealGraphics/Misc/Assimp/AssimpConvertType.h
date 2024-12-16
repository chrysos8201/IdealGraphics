#pragma once
#include "Core/Core.h"

#include "GraphicsEngine/VertexInfo.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "assimp/types.h"

namespace AssimpConvert
{
	struct ParticleMesh
	{
		std::string name;
		int32 boneIndex;	//뼈일 경우 인덱스 값
		std::string materialName;	// 이 mesh를 그릴때 필요한 material의 이름을 가지고 있겠다.

		std::vector<ParticleMeshVertex> vertices;
		std::vector<uint32> indices;
	};

	struct Mesh
	{
		std::string name;
		int32 boneIndex;	//뼈일 경우 인덱스 값
		std::string materialName;	// 이 mesh를 그릴때 필요한 material의 이름을 가지고 있겠다.

		std::vector<BasicVertex> vertices;
		std::vector<uint32> indices;

		Matrix localTM;

		// AABB 경계 정보
		Vector3 minBounds;  // AABB 최소 경계 (x, y, z)
		Vector3 maxBounds;  // AABB 최대 경계 (x, y, z)
	};

	struct SkinnedMesh
	{
		std::string name;
		int32 boneIndex = 0;
		std::string materialName;
		std::vector<SkinnedVertex> vertices;
		std::vector<uint32> indices;

		int32 parentIndexIfHaveNotBone = -1;
	};

	struct Bone
	{
		std::string name;
		int32 index = -1;
		int32 parent = -1;
		Matrix transform;
		Matrix offset;

		bool isNegative;
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
		std::string metalicTextureFile;
		std::string roughnessTextureFile;

		bool bUseDiffuseTexture;
		bool bUseNormalTexture;
		bool bUseMetallicTexture;
		bool bUseRoughnessTexture;

		float metallicFactor;
		float roughnessFactor;
	};


	// Animation

	struct BlendWeight
	{
		Vector4 indices = Vector4(0, 0, 0, 0);
		Vector4 weights = Vector4(0, 0, 0, 0);

		void Set(uint32 index, uint32 boneIndex, float weight)
		{
			float i = (float)boneIndex;
			
			switch (index)
			{
				case 0:
				{
					indices.x = i;
					weights.x = weight;
				}
				break;
				case 1:
				{
					indices.y = i;
					weights.y = weight;
				}
				break;
				case 2:
				{
					indices.z = i;
					weights.z = weight;
				}
				break;
				case 3:
				{
					indices.w = i;
					weights.w = weight;
				}
				break;
			}
		}
	};

	struct BoneWeights
	{
		void AddWeights(uint32 boneIndex, float weight)
		{
			if (weight <= 0.f)
			{
				return;
			}

			// 가중치가 더 크면 그 앞에 끼워 넣는다.
			auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
				[weight](const std::pair<int, float>& p) {return weight > p.second; });


			boneWeights.insert(findIt, { boneIndex, weight });
		}

		BlendWeight GetBlendWeights()
		{
			BlendWeight ret;
			for (uint32 i = 0; i < boneWeights.size(); ++i)
			{
				if (i >= 4)
					break;

				ret.Set(i, boneWeights[i].first, boneWeights[i].second);
			}

			return ret;
		}

		void Normalize()
		{
			// bone이 4보다 많을 경우
			if (boneWeights.size() >= 4)
			{
				boneWeights.resize(4);
			}

			float totalWeight = 0.f;
			for(const auto& item : boneWeights)
			{
				totalWeight += item.second;
			}

			// 비율을 맞춰줌
			float scale = 1.f / totalWeight;
			for (auto& item : boneWeights)
			{
				item.second *= scale;
			}
		}

		std::vector<std::pair<int32,float>> boneWeights;
	};

	struct KeyFrameData
	{
		float time;
		Vector3 scale;
		Quaternion rotation;
		Vector3 translation;
	};

	struct KeyFrame
	{
		std::string boneName;
		std::vector<KeyFrameData> transforms;
	};

	struct Animation
	{
		std::string name;
		uint32 frameCount = 0;
		float frameRate = 0.f;
		float duration= 0.f;
		int32 numBones = 0;
		
		std::vector<std::shared_ptr<KeyFrame>> keyFrames;
	};

	// cache
	struct AnimationNode
	{
		aiString name;	
		std::vector<KeyFrameData> keyframe;
	};
}

