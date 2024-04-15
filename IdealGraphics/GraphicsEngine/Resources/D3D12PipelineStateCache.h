#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"
#include "GraphicsEngine/Resources/D3D12Definitions.h"

namespace Ideal
{
	enum EPipelineStateInputLayout
	{
		ESimpleInputElement,
		EVertexElement,
		EPipelineStateInputLayoutCount
	};

	enum EPipelineStateVS
	{
		ESimpleVertexShaderVS,
		EPipelineStateVSCount
	};

	enum EPipelineStatePS
	{
		ESimplePixelShaderPS,
		ESimplePixelShaderPS2,
		EPipelineStatePSCount
	};

	struct ShaderData
	{
		LPCTSTR path;
		const char* entryPoint;
		const char* shaderVersion;
	};

	class D3D12PipelineStateCache
	{
	public:
		D3D12PipelineStateCache();
		virtual ~D3D12PipelineStateCache();

	public:
		void Create(ID3D12Device* Device, ID3D12RootSignature* RootSignature);
		ComPtr<ID3D12PipelineState> GetPipelineState(EPipelineStateInputLayout input, EPipelineStateVS vs, EPipelineStatePS ps);
	private:
		ComPtr<ID3D12PipelineState> m_pipelineStatesCache[EPipelineStateInputLayoutCount][EPipelineStateVSCount][EPipelineStatePSCount];

	private:
		static const D3D12_INPUT_ELEMENT_DESC PipelineStateInputLayouts[EPipelineStateInputLayoutCount][5];
		static const uint32 PipelineStateInputLayoutCount[EPipelineStateInputLayoutCount];
		static const ShaderData PipelineStateVSData[EPipelineStateVSCount];
		static const ShaderData PipelineStatePSData[EPipelineStatePSCount];
	};
}

