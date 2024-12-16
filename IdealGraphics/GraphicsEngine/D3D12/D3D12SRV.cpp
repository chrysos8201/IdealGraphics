#include "D3D12SRV.h"


Ideal::D3D12ShaderResourceView::D3D12ShaderResourceView()
{

}

Ideal::D3D12ShaderResourceView::~D3D12ShaderResourceView()
{
	m_handle.Free();
}

void Ideal::D3D12ShaderResourceView::Create(ID3D12Device* Device, ID3D12Resource* Resource, const Ideal::D3D12DescriptorHandle& Handle, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	Device->CreateShaderResourceView(Resource, &SRVDesc, Handle.GetCpuHandle());
	m_handle = Handle;
}

Ideal::D3D12DescriptorHandle Ideal::D3D12ShaderResourceView::GetHandle()
{
	return m_handle;
}

void Ideal::D3D12ShaderResourceView::SetResourceLocation(const Ideal::D3D12DescriptorHandle& handle)
{
	m_handle = handle;
}
