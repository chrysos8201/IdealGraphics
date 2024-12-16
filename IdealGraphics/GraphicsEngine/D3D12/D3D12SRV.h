#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	class D3D12DescriptorHandle;
}

namespace Ideal
{
	class D3D12ShaderResourceView
	{
	public:
		D3D12ShaderResourceView();
		~D3D12ShaderResourceView();

	public:
		void Create(ID3D12Device* Device, ID3D12Resource* Resource, const Ideal::D3D12DescriptorHandle& Handle, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
		Ideal::D3D12DescriptorHandle GetHandle();

		void SetResourceLocation(const Ideal::D3D12DescriptorHandle& handle);

	private:
		Ideal::D3D12DescriptorHandle m_handle;
	};
}