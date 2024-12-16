#include "Core/Core.h"

#include "D3D12PipelineStateObject.h"

#include "GraphicsEngine/D3D12/D3D12RootSignature.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"

Ideal::D3D12PipelineStateObject::D3D12PipelineStateObject()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
}

Ideal::D3D12PipelineStateObject::~D3D12PipelineStateObject()
{

}

void Ideal::D3D12PipelineStateObject::SetRootSignature(std::shared_ptr<Ideal::D3D12RootSignature> RootSignature)
{
	m_psoDesc.pRootSignature = RootSignature->GetRootSignature().Get();
}

void Ideal::D3D12PipelineStateObject::SetRootSignature(ID3D12RootSignature* RootSignature)
{
	m_psoDesc.pRootSignature = RootSignature;
}

void Ideal::D3D12PipelineStateObject::SetVertexShader(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_psoDesc.VS = Shader->GetShaderByteCode();
}

void Ideal::D3D12PipelineStateObject::SetPixelShader(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_psoDesc.PS = Shader->GetShaderByteCode();
}

void Ideal::D3D12PipelineStateObject::SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* InputElements, uint32 Count)
{
	m_psoDesc.InputLayout = { InputElements, Count };
}

void Ideal::D3D12PipelineStateObject::SetRasterizerState(const D3D12_RASTERIZER_DESC RSDesc /*= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)*/)
{
	m_psoDesc.RasterizerState = RSDesc;
	//m_psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//m_psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	//m_psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
}

void Ideal::D3D12PipelineStateObject::SetBlendState(const D3D12_BLEND_DESC BlendDesc /*= CD3DX12_BLEND_DESC(D3D12_DEFAULT)*/)
{
	m_psoDesc.BlendState = BlendDesc;
}

void Ideal::D3D12PipelineStateObject::SetTargetFormat(const uint32 RTVNum, DXGI_FORMAT* RTVFormat, const DXGI_FORMAT& DSVFormat)
{
	m_psoDesc.NumRenderTargets = RTVNum;

	for (uint32 i = 0; i < RTVNum; ++i)
	{
		m_psoDesc.RTVFormats[i] = RTVFormat[i];
	}
	m_psoDesc.DSVFormat = DSVFormat;
}

void Ideal::D3D12PipelineStateObject::Create(ID3D12Device* Device)
{
	//RootSignature
	//IA
	//VS
	//PS
	//RS
	//BS
	m_psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	m_psoDesc.SampleMask = UINT_MAX;
	m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//m_psoDesc.NumRenderTargets = 1;
	//m_psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//m_psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_psoDesc.SampleDesc.Count = 1;

	//Check(Device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf())));
	HRESULT hr = Device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf()));
	Check(hr);
}
