#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "ITexture.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include <memory>

struct ID3D12Resource;

namespace Ideal
{
	class D3D12Renderer;
	class DeferredDeleteManager;
}

namespace Ideal
{
	class D3D12Texture : public D3D12Resource, public ResourceBase,public ITexture, public std::enable_shared_from_this<D3D12Texture>
	{
	public:
		D3D12Texture();
		virtual ~D3D12Texture();

	public:
		//------------ITexture Interface------------//
		virtual uint64 GetImageID() override;
		virtual uint32 GetWidth() override;
		virtual uint32 GetHeight() override;

	public:
		// ResourceManager에서 호출된다.
		void Create(
			ComPtr<ID3D12Resource> Resource, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager
		);

		// DeferredDelete에 넣어주어야 한다.
		void Free();

		void EmplaceSRV(Ideal::D3D12DescriptorHandle SRVHandle);

		// 2024.05.15 Texture가 SRV, RTV, DTV기능을 하기위해 다 집어넣겠다.
		// RTV 만들 때 필요한 것. Resource가 있어야 하고
		// D3D12_RENDER_TARGET_VIEW_DESC 이 있네?
		void EmplaceRTV(Ideal::D3D12DescriptorHandle RTVHandle);

		void EmplaceDSV(Ideal::D3D12DescriptorHandle DSVHandle);

		void EmplaceUAV(Ideal::D3D12DescriptorHandle UAVHandle);
		// 2024.04.21 : 디스크립터가 할당된 위치를 가져온다.
		//D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() { return m_srvHandle.GetCpuHandle(); }
		Ideal::D3D12DescriptorHandle GetSRV();
		Ideal::D3D12DescriptorHandle GetRTV();
		Ideal::D3D12DescriptorHandle GetDSV();
		Ideal::D3D12DescriptorHandle GetUAV(uint32 i = 0);

		void SetUploadBuffer(ComPtr<ID3D12Resource> UploadBuffer, uint64 UploadBufferSize);
		ComPtr<ID3D12Resource> GetUploadBuffer() { return m_uploadBuffer; }
		void SetUpdated() { m_updated = true; }
		bool GetIsUpdated() { return m_updated; }

		void UpdateTexture(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList);

	public:
		void EmplaceSRVInEditor(Ideal::D3D12DescriptorHandle SRVHandle);

	private:
		Ideal::D3D12DescriptorHandle m_srvHandle;
		Ideal::D3D12DescriptorHandle m_rtvHandle;
		Ideal::D3D12DescriptorHandle m_dsvHandle;
		Ideal::D3D12DescriptorHandle m_uavHandle;

		Ideal::D3D12DescriptorHandle m_srvHandleInEditor;

		ComPtr<ID3D12Resource> m_uploadBuffer;
		uint64 m_uploadBufferSize = 0;

		std::weak_ptr<Ideal::DeferredDeleteManager> m_deferredDeleteManager;

		// UAVCount
		std::vector<Ideal::D3D12DescriptorHandle> m_uavHandles;
		uint32 m_uavCount = 0;

	private:
		// 2024.05.15 Texture일 경우 필요한 여러가지 정보들
		//std::vector<std::shared_ptr<Ideal::
		//std::vector<std::shared_ptr<Ideal::D3D12View>> m_renderTargetViews;

		uint32 m_width;
		uint32 m_height;

	private:
		uint32 m_refCount = 0;
		bool m_updated = false;
	};
}

