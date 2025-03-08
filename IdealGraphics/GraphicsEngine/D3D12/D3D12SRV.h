#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12Descriptors.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	class D3D12DescriptorHandle2;
}

namespace Ideal
{
	class D3D12ShaderResourceView
	{
	public:
		D3D12ShaderResourceView();
		~D3D12ShaderResourceView();

	public:
		void Create(ID3D12Device* Device, ID3D12Resource* Resource, const Ideal::D3D12DescriptorHandle2& Handle, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
		Ideal::D3D12DescriptorHandle2 GetHandle();

		void SetResourceLocation(const Ideal::D3D12DescriptorHandle2& handle);

	private:
		Ideal::D3D12DescriptorHandle2 m_handle;
	};
}