#include "GraphicsEngine/Resources/D3D12PipelineStateCache.h"

using namespace Ideal;
//std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>> PipelineStateInputLayouts;

std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>> PipelineStateInputLayouts = // = std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>>(2, std::vector<D3D12_INPUT_ELEMENT_DESC>());
{
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	}
};

LPCTSTR PipelineStateVSPath[] =
{
	L"Shaders/SimpleVertexShader.hlsl"
};

//std::wstring PipelineStatePSPath[] =
//{
//	L"Shaders/SimplePixelShader.hlsl",
//	L"Shaders/SimplePixelShader2.hlsl"
//};

std::vector<std::wstring> PipelineStatePSPath = 
{
	L"Shaders/SimplePixelShader.hlsl",
	L"Shaders/SimplePixelShader2.hlsl"
};

D3D12PipelineStateCache::D3D12PipelineStateCache()
{

}

D3D12PipelineStateCache::~D3D12PipelineStateCache()
{

}

void D3D12PipelineStateCache::Create(ID3D12Device* Device, ID3D12RootSignature* RootSignature)
{
	

	for (uint32 inputNum = 0; inputNum < EPipelineStateInputLayoutCount; ++inputNum)
	{
		for (uint32 vsNum = 0; vsNum < EPipelineStateVSCount; ++vsNum)
		{
			for (uint32 psNum = 0; psNum < EPipelineStatePSCount; ++psNum)
			{
				ComPtr<ID3DBlob> errorBlob;
				uint32 compileFlags = 0;
#if defined(_DEBUG)
				compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
				// VS
				ComPtr<ID3DBlob> vertexShader;
				Check(D3DCompileFromFile(PipelineStateVSPath[vsNum], nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));

				// PS
				ComPtr<ID3DBlob> pixelShader;
				Check(D3DCompileFromFile(PipelineStatePSPath[psNum].c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = { PipelineStateInputLayouts[inputNum].data(), static_cast<uint32>(PipelineStateInputLayouts[inputNum].size()) };
				psoDesc.pRootSignature = RootSignature;
				psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
				psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
				psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
				psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

				psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
				psoDesc.DepthStencilState.DepthEnable = FALSE;
				psoDesc.DepthStencilState.StencilEnable = FALSE;


				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.SampleDesc.Count = 1;

				Check(Device->CreateGraphicsPipelineState(
					&psoDesc,
					IID_PPV_ARGS(m_pipelineStatesCache[inputNum][vsNum][psNum].GetAddressOf())
				));
			}
		}
	}
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> D3D12PipelineStateCache::GetPipelineState(EPipelineStateInputLayout inputKey, EPipelineStateVS vsKey, EPipelineStatePS psKey)
{
	ComPtr ret = m_pipelineStatesCache[inputKey][vsKey][psKey];
	if (ret == nullptr)
	{
		assert(false);
		return nullptr;
	}
	return ret;
}
