#pragma once
#include "D3D12Common.h"
#include "d3d12.h"
#include "RHI\RHIMemoryPool.h"
#include <vector>

namespace Ideal { class D3D12PoolAllocator; }

namespace Ideal
{

	class D3D12DefaultBufferAllocator : public D3D12DeviceChild
	{
	
	public:
		void Begin(uint64 InFenceValue);
		void CleanupFreeBlocks(uint64 InFrameLag);

	public:
		D3D12DefaultBufferAllocator(ID3D12Device* InDevice);
		
		void AllocateDefaultResource(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, const D3D12_RESOURCE_DESC& InResourceDesc, D3D12_RESOURCE_STATES InInitialResourceState, ED3D12ResourceStateMode InResourceStateMode, D3D12ResourceLocation& ResourceLocation, uint32 Alignment);

	private:
		D3D12PoolAllocator* CreateBufferPool(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, D3D12_RESOURCE_STATES InInitialResourceState, uint32 Alignment);

	private:
		

		std::vector<D3D12PoolAllocator*> DefaultBufferPools;
	};
}
