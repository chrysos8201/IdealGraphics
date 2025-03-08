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
	class D3D12UnorderedAccessView
	{
	public:
		D3D12UnorderedAccessView();
		~D3D12UnorderedAccessView();

	public:
		void Create(ID3D12Device* Device, ID3D12Resource* Resource, const Ideal::D3D12DescriptorHandle2& Handle, const D3D12_UNORDERED_ACCESS_VIEW_DESC& UAVDesc);
		Ideal::D3D12DescriptorHandle2 GetHandle();

		void SetResourceLocation(const Ideal::D3D12DescriptorHandle2& handle);
		void SetResource(ComPtr<ID3D12Resource> Resource) { m_resource = Resource; }
		ComPtr<ID3D12Resource> GetResource() { return m_resource; }
	private:
		Ideal::D3D12DescriptorHandle2 m_handle;
		ComPtr<ID3D12Resource> m_resource;
	};
}
