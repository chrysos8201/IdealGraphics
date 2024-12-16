#pragma once
#include "IShader.h"
#include "Core/Core.h"
#include <d3dcommon.h>
#include <d3dx12.h>

namespace Ideal
{
	class D3D12Shader : public IShader
	{
	public:
		D3D12Shader();
		virtual ~D3D12Shader();

	public:
		void CompileFromFile(
			const std::wstring& Path,
			D3D_SHADER_MACRO* ShaderMacro,
			ID3DInclude* ShaderInclude,
			const std::string& Entry,
			const std::string& Version
		);
		// Compile from file
		const CD3DX12_SHADER_BYTECODE& GetShaderByteCode();
	private:
		ComPtr<ID3DBlob> m_shader;
		CD3DX12_SHADER_BYTECODE m_shaderByteCode;

	public:
		// DXC
		void AddShaderData(const std::vector<unsigned char>& byteCodes);
		void* GetBufferPointer() { return m_shaderCodes.data(); }
		uint32 GetSize() { return m_shaderCodes.size(); }

	private:
		std::vector<unsigned char> m_shaderCodes;
	};
}