#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

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

	enum EPipelineStateRS
	{
		ERasterizerStateSolid,
		ERasterizerStateWireFrame,
		EPipelineStateRSCount,
	};

	struct ShaderData
	{
		LPCTSTR path;
		const char* entryPoint;
		const char* shaderVersion;
	};

	class D3D12PipelineState
	{
	public:
		D3D12PipelineState();
		virtual ~D3D12PipelineState();

	public:
		void Create(ID3D12Device* Device, ID3D12RootSignature* RootSignature);
		ComPtr<ID3D12PipelineState> GetPipelineState(EPipelineStateInputLayout inputKey, EPipelineStateVS vsKey, EPipelineStatePS psKey, EPipelineStateRS rsKey);
	private:
		ComPtr<ID3D12PipelineState> m_pipelineStatesCache[EPipelineStateInputLayoutCount][EPipelineStateVSCount][EPipelineStatePSCount][EPipelineStateRSCount];

	private:
		static const D3D12_INPUT_ELEMENT_DESC PipelineStateInputLayouts[EPipelineStateInputLayoutCount][5];
		static const uint32 PipelineStateInputLayoutCount[EPipelineStateInputLayoutCount];
		static const ShaderData PipelineStateVSData[EPipelineStateVSCount];
		static const ShaderData PipelineStatePSData[EPipelineStatePSCount];
		static const D3D12_FILL_MODE PipelineStateRS[EPipelineStateRSCount];
	};
}

