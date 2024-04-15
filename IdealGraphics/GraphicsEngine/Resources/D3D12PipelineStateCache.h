#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"

namespace Ideal
{
	enum EPipelineStateInputLayout
	{
		ESimpleInputElement,
		EPipelineStateInputLayoutCount
	};
	//extern std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>> PipelineStateInputLayouts;
	
	enum EPipelineStateVS
	{
		ESimpleVertexShaderVS,
		EPipelineStateVSCount
	};
	//extern LPCTSTR PipelineStateVSPath[EPipelineStateVSCount];

	enum EPipelineStatePS
	{
		ESimplePixelShaderPS,
		ESimplePixelShaderPS2,
		EPipelineStatePSCount
	};
	//extern LPCTSTR PipelineStatePSPath[];

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
	};
}

