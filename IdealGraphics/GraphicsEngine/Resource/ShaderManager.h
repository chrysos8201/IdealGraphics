#pragma once
#include "Core/Core.h"
#include <dxcapi.h>

namespace Ideal
{
	class D3D12Shader;
}

namespace Ideal
{
	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();

	public:
		void Init();
		void CompileShader(const std::wstring& FilePath, const std::wstring& ShaderModel, ComPtr<IDxcBlob>& OutBlob);
		void CompileShader(const std::wstring& FilePath, const std::wstring& ShaderModel, const std::wstring& EntryPoint, ComPtr<IDxcBlob>& OutBlob);

		void CompileShaderAndSave(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName,
			const std::wstring& ShaderModel, ComPtr<IDxcBlob>& OutBlob, const std::wstring& EntryPoint = L"", bool HasEntry = false
		);
		
		void LoadShaderFile(const std::wstring& FilePath, std::shared_ptr<Ideal::D3D12Shader>& OutShader);
		void LoadShaderFile2(const std::wstring& FilePath, std::shared_ptr<Ideal::D3D12Shader>& OutShader);

		void AddIncludeDirectories(const std::wstring& IncludeDir) { m_includeDirectories.push_back(IncludeDir); }

	private:
		void ReadShaderFile(const std::wstring& FilePath, DxcBuffer* OutSourceBuffer, std::vector<char>& SourceData);
		void AddToArgumentsIncludeDirectories(std::vector<LPCWSTR>& Args);

		void ErrorCheck(ComPtr<IDxcResult> Result);
		void OutShader(ComPtr<IDxcBlob> Blob, const std::wstring& Path);
		void OutPDB(ComPtr<IDxcResult> Result, const std::wstring& Path);
		std::vector<std::wstring> m_includeDirectories;

	private:
		ComPtr<IDxcCompiler3> m_compiler;
		ComPtr<IDxcUtils> m_utils;
		ComPtr<IDxcIncludeHandler> m_includeHandler;
	};
}
