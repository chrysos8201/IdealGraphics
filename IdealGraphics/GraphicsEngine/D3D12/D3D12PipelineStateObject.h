#pragma once
#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include <d3d12.h>
#include <d3dx12.h>
namespace Ideal
{
	class D3D12RootSignature;
	class D3D12Shader;
	class D3D12Renderer;
}


namespace Ideal
{
	class D3D12PipelineStateObject
	{
	public:
		D3D12PipelineStateObject();
		virtual ~D3D12PipelineStateObject();

	public:
		void SetRootSignature	(std::shared_ptr<Ideal::D3D12RootSignature> RootSignature);
		void SetRootSignature	(ID3D12RootSignature* RootSignature);
		void SetVertexShader	(std::shared_ptr<Ideal::D3D12Shader> Shader);
		void SetPixelShader		(std::shared_ptr<Ideal::D3D12Shader> Shader);
		void SetInputLayout		(const D3D12_INPUT_ELEMENT_DESC* InputElements, uint32 Count);
		void SetRasterizerState (const D3D12_RASTERIZER_DESC RSDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		void SetBlendState		(const D3D12_BLEND_DESC BlendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		void SetTargetFormat(const uint32 RTVNum, DXGI_FORMAT* RTVFormat, const DXGI_FORMAT& DSVFormat);

		void Create (ID3D12Device* Device);

		ComPtr<ID3D12PipelineState> GetPipelineState() { return m_pipelineState; }

	private:
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
		ComPtr<ID3D12PipelineState> m_pipelineState;
	};
}