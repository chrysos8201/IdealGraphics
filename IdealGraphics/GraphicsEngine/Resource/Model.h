#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"

//class IdealRenderer;

namespace Ideal
{
	class Material;
	class Mesh;
	class Bone;
}

class ID3D12GraphicsCommandList;
class IdealRenderer;

namespace Ideal
{
	class Model : public ResourceBase
	{
	public:
		Model();
		virtual ~Model();
		
	public:
		void ReadMaterial(const std::wstring& filename);
		void ReadModel(const std::wstring& filename);

		// TEMP
		void Create(std::shared_ptr<IdealRenderer> Renderer);
		void Tick(uint32 FrameIndex);
		void Render(ID3D12GraphicsCommandList* CommandList, uint32 FrameIndex);

	private:
		void BindCacheInfo();

		std::shared_ptr<Ideal::Material> GetMaterialByName(const std::string& name);
		std::shared_ptr<Ideal::Mesh>	 GetMeshByName(const std::string& name);
		std::shared_ptr<Ideal::Bone>	 GetBoneByName(const std::string& name);

	private:
		std::vector<std::shared_ptr<Ideal::Material>> m_materials;
		std::vector<std::shared_ptr<Ideal::Bone>> m_bones;
		std::vector<std::shared_ptr<Ideal::Mesh>> m_meshes;
	};

}

