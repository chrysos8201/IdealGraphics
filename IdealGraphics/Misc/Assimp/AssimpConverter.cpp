#include "Misc/Assimp/AssimpConverter.h"

#include "Misc/Utils/tinyxml2.h"
#include "Misc/Utils/FileUtils.h"
#include <filesystem>

AssimpConverter::AssimpConverter()
	: m_scene(nullptr)
{
	m_importer = std::make_shared<Assimp::Importer>();
}

AssimpConverter::~AssimpConverter()
{

}

std::wstring AssimpConverter::ConvertStringToWString(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string AssimpConverter::ConvertWStringToString(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}

void AssimpConverter::Replace(std::string& OutStr, std::string Comp, std::string Rep)
{
	std::string temp = OutStr;
	size_t start_pos = 0;
	while ((start_pos = temp.find(Comp, start_pos)) != std::wstring::npos)
	{
		temp.replace(start_pos, Comp.length(), Rep);
		start_pos += Rep.length();
	}
	OutStr = temp;
}

void AssimpConverter::Replace(std::wstring& OutStr, std::wstring Comp, std::wstring Rep)
{
	std::wstring temp = OutStr;
	size_t start_pos = 0;
	while ((start_pos = temp.find(Comp, start_pos)) != std::wstring::npos)
	{
		temp.replace(start_pos, Comp.length(), Rep);
		start_pos += Rep.length();
	}
	OutStr = temp;
}

void AssimpConverter::ReadAssetFile(const std::wstring& file)
{
	std::wstring fileStr = m_assetPath + file;

	uint32 flag = 0;
	flag |= aiProcess_ConvertToLeftHanded;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_GenNormals;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_OptimizeMeshes;
	flag |= aiProcess_PreTransformVertices;

	m_scene = m_importer->ReadFile(
		ConvertWStringToString(fileStr),
		flag
	);

	if (!m_scene)
	{
		printf(m_importer->GetErrorString());
		assert(false);
	}

}

void AssimpConverter::ExportModelData(std::wstring savePath)
{
	std::wstring finalPath = m_modelPath + savePath + L".mesh";
	ReadModelData(m_scene->mRootNode, -1, -1);
	WriteModelFile(finalPath);
}

void AssimpConverter::ExportMaterialData(const std::wstring& savePath)
{
	std::wstring filePath = m_texturePath + savePath + L".xml";
	ReadMaterialData();
	WriteMaterialData(filePath);
}

std::string AssimpConverter::WriteTexture(std::string SaveFolder, std::string File)
{
	std::string fileName = std::filesystem::path(File).filename().string();
	std::string folderName = std::filesystem::path(SaveFolder).filename().string();

	const aiTexture* srcTexture = m_scene->GetEmbeddedTexture(File.c_str());

	// 텍스쳐가 내장되어있을 경우
	if (srcTexture)
	{
		std::string pathStr = SaveFolder + fileName;

		if (srcTexture->mHeight == 0)
		{
			std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
			file->Open(ConvertStringToWString(pathStr), FileMode::Write);
			file->Write(srcTexture->pcData, srcTexture->mWidth);
		}
	}
	// 텍스쳐가 따로 있을 경우
	else
	{
		// 원래있던 파일을 내가 원하는 경로로 옮긴다.
		if (File.size() > 0)
		{

			std::string originStr = (std::filesystem::path(m_assetPath) / folderName / File).string();
			Replace(originStr, "\\", "/");

			std::string pathStr = (std::filesystem::path(SaveFolder) / fileName).string();
			Replace(pathStr, "\\", "/");

			bool isSuccess = ::CopyFileA(originStr.c_str(), pathStr.c_str(), false);
			if (!isSuccess)
			{
				assert(false);
			}
		}
	}
	return fileName;
}

void AssimpConverter::WriteMaterialData(std::wstring FilePath)
{
	auto path = std::filesystem::path(FilePath);

	std::filesystem::create_directory(path.parent_path());

	std::string folder = path.parent_path().string();

	std::shared_ptr<tinyxml2::XMLDocument> document = std::make_shared<tinyxml2::XMLDocument>(); 

	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	tinyxml2::XMLElement* root = document->NewElement("Materials");
	document->LinkEndChild(root);

	for (std::shared_ptr<AssimpConvert::Material> material : m_materials)
	{
		tinyxml2::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);

		tinyxml2::XMLElement* element = nullptr;

		element = document->NewElement("Name");
		element->SetText(material->name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->diffuseTextureFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->specularTextureFile).c_str());
		node->LinkEndChild(element);
		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->normalTextureFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->ambient.x);
		element->SetAttribute("G", material->ambient.y);
		element->SetAttribute("B", material->ambient.z);
		element->SetAttribute("A", material->ambient.w);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->diffuse.x);
		element->SetAttribute("G", material->diffuse.y);
		element->SetAttribute("B", material->diffuse.z);
		element->SetAttribute("A", material->diffuse.w);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->specular.x);
		element->SetAttribute("G", material->specular.y);
		element->SetAttribute("B", material->specular.z);
		element->SetAttribute("A", material->specular.w);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->emissive.x);
		element->SetAttribute("G", material->emissive.y);
		element->SetAttribute("B", material->emissive.z);
		element->SetAttribute("A", material->emissive.w);
		node->LinkEndChild(element);
	}

	std::string filePathString = std::string().assign(FilePath.begin(), FilePath.end());
	document->SaveFile(filePathString.c_str());
}

void AssimpConverter::WriteModelFile(const std::wstring& filePath)
{
	auto path = std::filesystem::path(filePath);

	std::filesystem::create_directory(path.parent_path());

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(filePath, FileMode::Write);

	// Bone Data
	file->Write<uint32>(m_bones.size());
	for(auto& bone : m_bones)
	{
		file->Write<int32>(bone->index);
		file->Write<std::string>(bone->name);
		file->Write<int32>(bone->parent);
		file->Write<Matrix>(bone->transform);
	}
	file->Write<uint32>(m_meshes.size());
	for (auto& mesh : m_meshes)
	{
		file->Write<std::string>(mesh->name);
		file->Write<int32>(mesh->boneIndex);
		file->Write<std::string>(mesh->materialName);

		// vertex
		file->Write<uint32>(mesh->vertices.size());
		file->Write(&mesh->vertices[0], sizeof(BasicVertex)* mesh->vertices.size());

		// index
		file->Write<uint32>(mesh->indices.size());
		file->Write(&mesh->indices[0], sizeof(uint32) * mesh->indices.size());

	}
}

void AssimpConverter::ReadModelData(aiNode* node, int32 index, int32 parent)
{
	std::shared_ptr<AssimpConvert::Bone> bone = std::make_shared<AssimpConvert::Bone>();
	bone->index = index;
	bone->parent = parent;
	bone->name = node->mName.C_Str();


	/// Relative Transform
	// float 첫번째 주소 값으로 matrix 복사
	Matrix transform(node->mTransformation[0]);
	bone->transform = transform.Transpose();

	// root (local)
	Matrix matParent = Matrix::Identity;
	if (parent >= 0)
		matParent = m_bones[parent]->transform;

	// Local Transform
	bone->transform = bone->transform * matParent;

	m_bones.push_back(bone);

	// Mesh
	ReadMeshData(node, index);

	for (uint32 i = 0; i < node->mNumChildren; ++i)
	{
		ReadModelData(node->mChildren[i], m_bones.size(), index);
	}
}

void AssimpConverter::ReadMaterialData()
{
	for (uint32 i = 0; i < m_scene->mNumMaterials; ++i)
	{
		aiMaterial* srcMaterial = m_scene->mMaterials[i];

		std::shared_ptr<AssimpConvert::Material> material = std::make_shared<AssimpConvert::Material>();
		material->name = srcMaterial->GetName().C_Str();
		
		aiColor3D color;

		// Ambient
		srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->ambient = Color(color.r, color.g, color.b, 1.f);

		// Diffuse
		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->diffuse = Color(color.r, color.g, color.b, 1.f);

		// Specular
		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->specular = Color(color.r, color.g, color.b, 1.f);
		srcMaterial->Get(AI_MATKEY_SHININESS, material->specular.w);

		// Emissive
		srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->emissive = Color(color.r, color.g, color.b, 1.f);

		//----------------Texture----------------//
		aiString file;
		/*if (srcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), file) == AI_SUCCESS)
		{
			int a = 3;
		}*/
		// Diffuse Texture
		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		material->diffuseTextureFile = file.C_Str();

		// Specular Texture
		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->specularTextureFile = file.C_Str();

		// Emissive Texture
		srcMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &file);
		material->emissiveTextureFile = file.C_Str();

		// Normal Texture
		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->normalTextureFile = file.C_Str();

		m_materials.push_back(material);
	}
}

void AssimpConverter::ReadMeshData(aiNode* node, int32 bone)
{
	// 마지막 노드는 정보를 들고 있다.
	if (node->mNumMeshes < 1)
	{
		return;
	}

	std::shared_ptr<AssimpConvert::Mesh> mesh = std::make_shared<AssimpConvert::Mesh>();
	mesh->name = node->mName.C_Str();
	mesh->boneIndex = bone;

	// submesh
	for (uint32 i = 0; i < node->mNumMeshes; ++i)
	{
	 	uint32 index = node->mMeshes[i];
		const aiMesh* srcMesh = m_scene->mMeshes[index];

		// Material 이름
		const aiMaterial* material = m_scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();

		// mesh가 여러개일 경우 index가 중복될 수 있다. 
		// 하나로 관리하기 위해 미리 이전 vertex의 size를 가져와서 이번에 추가하는 index에 더해 중복을 피한다.

		const uint32 startVertex = mesh->vertices.size();

		// Vertex
		for (uint32 v = 0; v < srcMesh->mNumVertices; ++v)
		{
			BasicVertex vertex;
			{
				memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3));
			}

			// UV
			if (srcMesh->HasTextureCoords(0))
			{
				memcpy(&vertex.UV, &srcMesh->mTextureCoords[0][v],sizeof(Vector2));
			}

			// Normal
			if (srcMesh->HasNormals())
			{
				memcpy(&vertex.Normal, &srcMesh->mNormals[v], sizeof(Vector3));
			}

			mesh->vertices.push_back(vertex);
		}

		// Index
		for (uint32 f = 0; f < srcMesh->mNumFaces; ++f)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (uint32 k = 0; k < face.mNumIndices; ++k)
			{
				mesh->indices.push_back(face.mIndices[k] + startVertex);
			}
		}
	}
	m_meshes.push_back(mesh);
}
