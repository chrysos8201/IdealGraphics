#include "D3D12UAV.h"

Ideal::D3D12UnorderedAccessView::D3D12UnorderedAccessView()
{

}

Ideal::D3D12UnorderedAccessView::~D3D12UnorderedAccessView()
{
	m_handle.Free();
}

void Ideal::D3D12UnorderedAccessView::Create(ID3D12Device* Device, ID3D12Resource* Resource, const Ideal::D3D12DescriptorHandle2& Handle, const D3D12_UNORDERED_ACCESS_VIEW_DESC& UAVDesc)
{
	Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, Handle.GetCPUDescriptorHandleStart());
	m_handle = Handle;
}

Ideal::D3D12DescriptorHandle2 Ideal::D3D12UnorderedAccessView::GetHandle()
{
	return m_handle;
}

void Ideal::D3D12UnorderedAccessView::SetResourceLocation(const Ideal::D3D12DescriptorHandle2& handle)
{
	m_handle = handle;
}
