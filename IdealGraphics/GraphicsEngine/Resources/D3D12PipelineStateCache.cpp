#include "GraphicsEngine/Resources/D3D12PipelineStateCache.h"

using namespace Ideal;

const D3D12_INPUT_ELEMENT_DESC D3D12PipelineStateCache::PipelineStateInputLayouts[][5] =
{
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	},
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	}
};

// InputLayouts가 가지고 있는 Element집합 개별이 가지고 있는 Element의 개수.......
const uint32 D3D12PipelineStateCache::PipelineStateInputLayoutCount[EPipelineStateInputLayoutCount] =
{
	2,
	5
};

const ShaderData D3D12PipelineStateCache::PipelineStateVSData[EPipelineStateVSCount] =
{
	{
		L"Shaders/SimpleVertexShader.hlsl",
		"main",
		"vs_5_0"
	},
};

const ShaderData D3D12PipelineStateCache::PipelineStatePSData[EPipelineStatePSCount] =
{
	{
		L"Shaders/SimplePixelShader.hlsl",
		"main",
		"ps_5_0"
	},
	{
		L"Shaders/SimplePixelShader2.hlsl",
		"main",
		"ps_5_0"
	}
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
				Check(D3DCompileFromFile(PipelineStateVSData[vsNum].path,
					nullptr, 
					nullptr,
					PipelineStateVSData[vsNum].entryPoint,
					PipelineStateVSData[vsNum].shaderVersion,
					compileFlags, 0, &vertexShader, nullptr));

				// PS
				ComPtr<ID3DBlob> pixelShader;
				Check(D3DCompileFromFile(PipelineStatePSData[psNum].path,
					nullptr, 
					nullptr, 
					PipelineStatePSData[psNum].entryPoint,
					PipelineStatePSData[psNum].shaderVersion,
					compileFlags, 0, &pixelShader, nullptr));

				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = { PipelineStateInputLayouts[inputNum], PipelineStateInputLayoutCount[inputNum]};

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

				WCHAR name[50];
				if (swprintf_s(name, L"m_pipelineStates[%d][%d][%d]", inputNum, vsNum, psNum) > 0)
				{
					//SetName(pLibrary->m_pipelineStates[type].Get(), name);
					m_pipelineStatesCache[inputNum][vsNum][psNum]->SetName(name);
				}
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
