#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "ThirdParty/Include/DirectXTK12/WICTextureLoader.h"
#include "GraphicsEngine/D3D12/D3D12Renderer.h"
#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"
#include <d3d12.h>
#include <d3dx12.h>

Ideal::D3D12Texture::D3D12Texture()
{

}

Ideal::D3D12Texture::~D3D12Texture()
{
	// 2024.11.06 : 뭔가 리사이즈 할 때 죽는다.
	m_srvHandle.Free();
	m_rtvHandle.Free();
	m_dsvHandle.Free();
	//for (uint32 i = 0; i < m_uavHandles.size(); ++i)
	//{
	//	m_uavHandles[i].Free();
	//}
	//m_uavHandles.clear();
	m_uavHandle.Free();
	m_srvHandleInEditor.Free();
}

uint64 Ideal::D3D12Texture::GetImageID()
{
	return static_cast<uint64>(m_srvHandleInEditor.GetGpuHandle().ptr);
}

uint32 Ideal::D3D12Texture::GetWidth()
{
	uint64 ret = m_resource->GetDesc().Width;
	return (uint32)ret;
}

uint32 Ideal::D3D12Texture::GetHeight()
{
	uint64 ret = m_resource->GetDesc().Height;
	return (uint32)ret;
}

void Ideal::D3D12Texture::Create(ComPtr<ID3D12Resource> Resource, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager)
{
	m_resource = Resource;
	m_deferredDeleteManager = DeferredDeleteManager;
}

void Ideal::D3D12Texture::Free()
{
	m_srvHandle.Free();
	m_rtvHandle.Free();
	m_dsvHandle.Free();
	//m_uavHandle.Free();
	//for (uint32 i = 0; i < m_uavHandles.size(); ++i)
	//{
	//	m_uavHandles[i].Free();
	//}
	//m_uavHandles.clear();
	m_uavHandle.Free();
	m_srvHandleInEditor.Free();
}

void Ideal::D3D12Texture::EmplaceDSV(Ideal::D3D12DescriptorHandle DSVHandle)
{
	m_dsvHandle = DSVHandle;
}

Ideal::D3D12DescriptorHandle Ideal::D3D12Texture::GetDSV()
{
	if (m_dsvHandle.GetCpuHandle().ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		__debugbreak();
	}
	return m_dsvHandle;
}
void Ideal::D3D12Texture::SetUploadBuffer(ComPtr<ID3D12Resource> UploadBuffer, uint64 UploadBufferSize)
{
	m_uploadBuffer = UploadBuffer;
	m_uploadBufferSize = UploadBufferSize;
}

void Ideal::D3D12Texture::UpdateTexture(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList)
{
	if (GetIsUpdated() == true)
	{
		m_updated = false;
		if (m_uploadBuffer == nullptr) return;	// Is Not Dynamic Texture

		const DWORD MAX_SUB_RESOURCE_NUM = 32;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint[MAX_SUB_RESOURCE_NUM] = {};
		uint32 rows[MAX_SUB_RESOURCE_NUM] = {};
		uint64 rowSize[MAX_SUB_RESOURCE_NUM] = {};
		uint64 totalBytes = 0;

		D3D12_RESOURCE_DESC Desc = m_resource->GetDesc();
		if (Desc.MipLevels > (uint32)_countof(footPrint))
		{
			__debugbreak();
		}

		Device->GetCopyableFootprints(&Desc, 0, Desc.MipLevels, 0, footPrint, rows, rowSize, &totalBytes);
		CD3DX12_RESOURCE_BARRIER ShaderResourceToCopyDestBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		CommandList->ResourceBarrier(1, &ShaderResourceToCopyDestBarrier);

		//BYTE* data;
		//m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));
		//ZeroMemory(data, static_cast<size_t>(m_uploadBufferSize));
		//m_uploadBuffer->Unmap(0, nullptr);

		for (DWORD i = 0; i < Desc.MipLevels; ++i)
		{

			D3D12_TEXTURE_COPY_LOCATION destLocation = {};
			destLocation.PlacedFootprint = footPrint[i];
			destLocation.pResource = m_resource.Get();
			destLocation.SubresourceIndex = i;
			destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.PlacedFootprint = footPrint[i];
			srcLocation.pResource = m_uploadBuffer.Get();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

			CommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
		}

		CD3DX12_RESOURCE_BARRIER copyToShaderResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
		);
		CommandList->ResourceBarrier(1, &copyToShaderResourceBarrier);
	}
}

void Ideal::D3D12Texture::EmplaceSRVInEditor(Ideal::D3D12DescriptorHandle SRVHandle)
{
	m_srvHandleInEditor = SRVHandle;
}

