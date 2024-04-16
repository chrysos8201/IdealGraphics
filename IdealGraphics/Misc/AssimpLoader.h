#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Mesh.h"
struct Mesh;
struct Vertex;

struct aiMesh;
struct aiMaterial;

class MeshTest;
class ModelTest;

namespace Ideal
{
	class Mesh;
}
struct ImportSettings
{
	std::string filename;
	std::vector<Mesh>& meshes;
	bool inverseU = false;
	bool inverseV = false;
};
struct ImportSettings2
{
	std::string filename;
	std::vector<MeshTest>& meshes;
	bool inverseU = false;
	bool inverseV = false;
};

struct ImportSettings3
{
	std::string filename;
	std::vector<std::shared_ptr<Ideal::Mesh>>& meshes;
	bool inverseU = false;
	bool inverseV = false;
};

class AssimpLoader
{
public:
	//bool Load(ImportSettings setting);

private:
	//void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);
	//void LoadTexture(const std::string& filename, Mesh& dst, const aiMaterial* src);

public:
	//bool Load(ImportSettings2 setting);

private:
	//void LoadMesh(MeshTest& dst, const aiMesh* src, bool inverseU, bool inverseV);
	//void LoadTexture(const std::string& filename, MeshTest& dst, const aiMaterial* src);

public:
	bool Load(ImportSettings3 settings);
	
private:
	void LoadMesh(std::shared_ptr<Ideal::Mesh> mesh, const aiMesh* src, bool inverseU, bool inverseV);
	void LoadTexture(const std::string& filename, std::shared_ptr<Ideal::Mesh> dst, const aiMaterial* src);
};