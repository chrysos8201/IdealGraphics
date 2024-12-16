#pragma once
#include "Core/Core.h"
//#include "d3dx12.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	class D3D12RootSignature
	{
	public:
		D3D12RootSignature();
		virtual ~D3D12RootSignature();

	public:
		void Create(uint32 NumRootParams, uint32 NumStaticSamplers);
		ComPtr<ID3D12RootSignature> GetRootSignature() { return m_rootSignature; }

		//void InitAsConstantBufferView(uint32 Index, );
	public:
		std::vector<CD3DX12_ROOT_PARAMETER1> m_rootParameters;
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}