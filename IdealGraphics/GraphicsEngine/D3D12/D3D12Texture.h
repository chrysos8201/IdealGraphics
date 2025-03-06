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
	class D3D12Texture : public D3D12Resource, public ResourceBase, public ITexture, public std::enable_shared_from_this<D3D12Texture>
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
		void EmplaceDSV(Ideal::D3D12DescriptorHandle DSVHandle);

		Ideal::D3D12DescriptorHandle GetDSV();
		

		void SetUploadBuffer(ComPtr<ID3D12Resource> UploadBuffer, uint64 UploadBufferSize);
		ComPtr<ID3D12Resource> GetUploadBuffer() { return m_uploadBuffer; }
		void SetUpdated() { m_updated = true; }
		bool GetIsUpdated() { return m_updated; }

		void UpdateTexture(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList);

	public:
		void EmplaceSRVInEditor(Ideal::D3D12DescriptorHandle SRVHandle);

	private:
		Ideal::D3D12DescriptorHandle m_dsvHandle;
		Ideal::D3D12DescriptorHandle m_srvHandleInEditor;

		ComPtr<ID3D12Resource> m_uploadBuffer;
		uint64 m_uploadBufferSize = 0;
		std::weak_ptr<Ideal::DeferredDeleteManager> m_deferredDeleteManager;

	private:

		uint32 m_width;
		uint32 m_height;

	private:
		uint32 m_refCount = 0;
		bool m_updated = false;
	};
}

