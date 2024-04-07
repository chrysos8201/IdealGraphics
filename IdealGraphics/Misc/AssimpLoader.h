#pragma once
#include "Core/Core.h"

struct Mesh;
struct Vertex;

struct aiMesh;
struct aiMaterial;

class MeshTest;
class ModelTest;

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
class AssimpLoader
{
public:
	bool Load(ImportSettings setting);

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);
	void LoadTexture(const std::string& filename, Mesh& dst, const aiMaterial* src);

public:
	bool Load(ImportSettings2 setting);

private:
	void LoadMesh(MeshTest& dst, const aiMesh* src, bool inverseU, bool inverseV);
	void LoadTexture(const std::string& filename, MeshTest& dst, const aiMaterial* src);
};

