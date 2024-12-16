#include "GraphicsEngine/Resource/ShaderManager.h"
#include "Misc/Utils/FileUtils.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include <fstream>
#include <filesystem>

using namespace Ideal;

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{

}

void ShaderManager::Init()
{
	Check(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)), L"Failed To Create IDxcCompiler3");
	Check(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)), L"Failed To Create IDxcUtils");
	Check(m_utils->CreateDefaultIncludeHandler(&m_includeHandler), L"Failed To Create IDxcIncludeHandler");
}

void ShaderManager::CompileShader(const std::wstring& FilePath, const std::wstring& ShaderModel, ComPtr<IDxcBlob>& OutBlob)
{
	// Read Shader
	std::vector<char> sourceData;
	DxcBuffer sourceBuffer;
	ReadShaderFile(FilePath, &sourceBuffer, sourceData);

	std::vector<LPCWSTR> args;
	std::wstring outputPath;
	std::wstring pdbPath;
	// Debug pdb
	{

		std::wstring path = FilePath;
		path.pop_back();
		path.pop_back();
		path.pop_back();
		path.pop_back();
		path.pop_back();

		// 출력 파일 경로 설정
		outputPath = path + L".cso";
		// PDB 파일 경로 설정
		pdbPath = path + L".pdb";
		args.push_back(L"-Zi");
#ifdef _DEBUG
		args.push_back(L"-Od");	// 최적화 끄기
#endif
		/*args.push_back(L"-Fo");
		args.push_back(outputPath.c_str());*/

		args.push_back(L"-Fd");
		args.push_back(pdbPath.c_str());
	}
	// Shader Model
	{
		args.push_back(L"-T");
		args.push_back(ShaderModel.c_str());
	}
	// Include Directories
	{
		AddToArgumentsIncludeDirectories(args);
	}
	ComPtr<IDxcResult> result;
	m_compiler->Compile(&sourceBuffer, args.data(), args.size(), m_includeHandler.Get(), IID_PPV_ARGS(&result));

	// ERROR CHECK
	ErrorCheck(result);
	OutPDB(result, pdbPath);
	// Get Blob
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&OutBlob), nullptr);
}

void ShaderManager::CompileShader(const std::wstring& FilePath, const std::wstring& ShaderModel, const std::wstring& EntryPoint, ComPtr<IDxcBlob>& OutBlob)
{
	// Read Shader
	DxcBuffer sourceBuffer;
	std::vector<char> sourceData;
	ReadShaderFile(FilePath, &sourceBuffer, sourceData);

	std::vector<LPCWSTR> args;
	std::wstring pdbPath;
	// Debug pdb
	{

		std::wstring path = FilePath;
		path.pop_back();
		path.pop_back();
		path.pop_back();
		path.pop_back();
		path.pop_back();
		// PDB 파일 경로 설정
		pdbPath = path + L".pdb";
		args.push_back(L"-Zi");
#ifdef _DEBUG
		args.push_back(L"-Od");	// 최적화 끄기
#endif
		args.push_back(L"-Fd");
		args.push_back(pdbPath.c_str());
	}
	// Entry Point
	{
		args.push_back(L"-E");
		args.push_back(EntryPoint.c_str());
	}
	// Shader Model
	{
		args.push_back(L"-T");
		args.push_back(ShaderModel.c_str());
	}
	// Include Directories
	{
		AddToArgumentsIncludeDirectories(args);
	}
	ComPtr<IDxcResult> result;
	m_compiler->Compile(&sourceBuffer, args.data(), args.size(), m_includeHandler.Get(), IID_PPV_ARGS(&result));

	// ERROR CHECK
	ErrorCheck(result);
	OutPDB(result, pdbPath);
	// Get Blob
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&OutBlob), nullptr);
}

void Ideal::ShaderManager::CompileShaderAndSave(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName, const std::wstring& ShaderModel, ComPtr<IDxcBlob>& OutBlob, const std::wstring& EntryPoint /*= L""*/, bool HasEntry /*= false*/)
{
	if (HasEntry)
		CompileShader(FilePath, ShaderModel, EntryPoint, OutBlob);
	else
		CompileShader(FilePath, ShaderModel, OutBlob);

	//OutShader(OutBlob, SavePath + SaveName + L".cso");

	//
	uint32 Size = OutBlob->GetBufferSize();
	void* pointer = OutBlob->GetBufferPointer();
	//
	std::wstring finalPath = SavePath + SaveName + L".shader";
	auto path = std::filesystem::path(finalPath);
	std::filesystem::create_directory(path.parent_path());
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(finalPath, FileMode::Write);

	file->Write<uint32>(Size);
	// buffer size
	file->Write(pointer, Size);

	// buffer data
}

void Ideal::ShaderManager::LoadShaderFile(const std::wstring& FilePath, std::shared_ptr<Ideal::D3D12Shader>& OutShader)
{
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(FilePath, FileMode::Read);

	const uint32 size = file->Read<uint32>();

	std::vector<unsigned char> shader;
	shader.resize(size);
	void* data = shader.data();
	file->Read(&data, sizeof(unsigned char) * size);

	if (!OutShader)
	{
		OutShader = std::make_shared<Ideal::D3D12Shader>();
	}
	OutShader->AddShaderData(shader);
}

void ShaderManager::LoadShaderFile2(const std::wstring& FilePath, std::shared_ptr<Ideal::D3D12Shader>& OutShader)
{
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(FilePath, FileMode::Read);

	const uint32 size = file->Read<uint32>();

	std::vector<unsigned char> shader;
	shader.resize(size);
	void* data = shader.data();
	file->Read(&data, sizeof(unsigned char) * size);

	if (!OutShader)
	{
		OutShader = std::make_shared<Ideal::D3D12Shader>();
	}
	OutShader->AddShaderData(shader);
}

void Ideal::ShaderManager::ReadShaderFile(const std::wstring& FilePath, DxcBuffer* OutSourceBuffer, std::vector<char>& SourceData)
{
	std::ifstream shaderFile(FilePath, std::ios::binary);
	SourceData = std::vector<char>((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

	// hlsl compile
	OutSourceBuffer->Ptr = SourceData.data();
	OutSourceBuffer->Size = SourceData.size();
	OutSourceBuffer->Encoding = DXC_CP_UTF8;
}

void ShaderManager::OutShader(ComPtr<IDxcBlob> Blob, const std::wstring& Path)
{
	std::ofstream shaderFile(Path, std::ios::binary);
	shaderFile.write(reinterpret_cast<const char*>(Blob->GetBufferPointer()), Blob->GetBufferSize());
	shaderFile.close();
}

void Ideal::ShaderManager::OutPDB(ComPtr<IDxcResult> Result, const std::wstring& Path)
{
	ComPtr<IDxcBlob> pdbBlob;
	HRESULT hr = Result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), nullptr);
	if (FAILED(hr) || pdbBlob == nullptr)
	{
		// Handle the error
		return;
	}

	// PDB 파일을 디스크에 저장
	std::ofstream pdbFile(Path, std::ios::binary);
	pdbFile.write(reinterpret_cast<const char*>(pdbBlob->GetBufferPointer()), pdbBlob->GetBufferSize());
	pdbFile.close();
}

void ShaderManager::AddToArgumentsIncludeDirectories(std::vector<LPCWSTR>& Args)
{
	for (uint32 i = 0; i < m_includeDirectories.size(); ++i)
	{
		Args.push_back(L"-I");
		Args.push_back(m_includeDirectories[i].c_str());
	}
}

void ShaderManager::ErrorCheck(ComPtr<IDxcResult> Result)
{
	ComPtr<IDxcBlobEncoding> encoding = nullptr;
	Result->GetErrorBuffer(&encoding);
	const char* errorMessage = (char*)encoding->GetBufferPointer();
	if (errorMessage)
	{
		MyMessageBox(errorMessage);
	}
}
