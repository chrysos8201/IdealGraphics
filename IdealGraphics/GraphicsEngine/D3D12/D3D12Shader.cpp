//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "D3D12Shader.h"
#include <d3dcompiler.h>

Ideal::D3D12Shader::D3D12Shader()
{

}

Ideal::D3D12Shader::~D3D12Shader()
{

}

void Ideal::D3D12Shader::CompileFromFile(const std::wstring& Path, D3D_SHADER_MACRO* ShaderMacro, ID3DInclude* ShaderInclude, const std::string& Entry, const std::string& Version)
{
	uint32 compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ComPtr<ID3DBlob> errorBlob;

	D3DCompileFromFile(
		Path.c_str(),
		ShaderMacro,
		ShaderInclude,
		Entry.c_str(),
		Version.c_str(),
		compileFlags,
		0,
		m_shader.GetAddressOf(),
		errorBlob.GetAddressOf()
	);

	if (errorBlob.Get() != nullptr)
	{
		const char* errorMsg = (char*)errorBlob->GetBufferPointer();
		MessageBoxA(NULL, errorMsg, "ERROR", MB_OK);
		assert(false);
	}

	m_shaderByteCode = CD3DX12_SHADER_BYTECODE(m_shader.Get());
}

const CD3DX12_SHADER_BYTECODE& Ideal::D3D12Shader::GetShaderByteCode()
{
	return m_shaderByteCode;
}

void Ideal::D3D12Shader::AddShaderData(const std::vector<unsigned char>& byteCodes)
{
	m_shaderCodes.insert(m_shaderCodes.end(), byteCodes.begin(), byteCodes.end());
	m_shaderByteCode = CD3DX12_SHADER_BYTECODE(m_shaderCodes.data(), m_shaderCodes.size());
}
