#pragma once
#include "Core/Core.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	struct UploadBufferContainer
	{
		D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;
		uint8* SystemMemoryAddress;
	};

	class D3D12UploadBufferPool
	{
	public:
		D3D12UploadBufferPool();
		virtual ~D3D12UploadBufferPool();

	public:
		void Init(ID3D12Device* Device, uint32 Offset, uint32 MaxInstanceNum);
		std::shared_ptr<UploadBufferContainer> Allocate();
		void Reset();

		ComPtr<ID3D12Resource> GetResource() { return m_resource; }

	private:
		ComPtr<ID3D12Resource> m_resource;
		uint32 m_offset;
		uint32 m_maxInstanceNum;
		uint8* m_systemMemAddr;

		std::vector<std::shared_ptr<UploadBufferContainer>> m_uploadBufferContainers;
		D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress;

	private:
		uint32 m_allocatedInstanceNum;
	};
}