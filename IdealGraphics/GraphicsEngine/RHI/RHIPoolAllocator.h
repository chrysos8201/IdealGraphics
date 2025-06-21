#pragma once
#include "Core\Core.h"
#include "D3D12\D3D12Common.h"
#include "d3d12.h"

using namespace Ideal;


namespace Ideal { struct RHIPoolAllocationData; }

namespace Ideal { class RHIMemoryPool; }

struct ID3D12Device;

namespace Ideal
{
	class RHIPoolAllocator
	{
	public:
		RHIPoolAllocator(uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled);

	protected:
		bool TryAllocateInternal(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& InAllocationData);
		virtual RHIMemoryPool* CreateNewPool(uint32 InPoolIndex, uint32 InMinimumAllocationSize) = 0;

		std::vector<RHIMemoryPool*> Pools;

		const uint64 DefaultPoolSize;
		const uint32 PoolAlignment;
		const uint64 MaxAllocationSize;
		const bool bDefragEnabled;

		// Pool을 정렬
		std::vector<uint32> PoolAllocationOrder;
	};

	// Constant Buffer의 Alignment는 256U이다. 다만 리소스가 최소 64KB인 것일 뿐...
	const uint32 D3D12ManualSubAllocationAlignment = 256U;

	class D3D12PoolAllocator : public RHIPoolAllocator, public D3D12DeviceChild
	{
	public:
		static EResourceAllocationStrategy GetResourceAllocationStrategy(D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment);

	public:
		D3D12PoolAllocator(ComPtr<ID3D12Device> InDevice, const D3D12ResourceInitConfig& InInitConfig, EResourceAllocationStrategy InAllocationStrategy, uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled);

		bool SupportsAllocation(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, D3D12_RESOURCE_STATES InInitialResourceState, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment) const;
		void AllocateDefaultResource(D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& InResourceDesc, const D3D12_RESOURCE_STATES InResourceCreateState, ED3D12ResourceStateMode InResourceStateMode, uint32 InAllocationAlignment, Ideal::D3D12ResourceLocation& ResourceLocation);

	protected:
		virtual RHIMemoryPool* CreateNewPool(uint32 InPoolIndex, uint32 InMinimumAllocationSize) override;

		EResourceAllocationStrategy AllocationStrategy;
		D3D12ResourceInitConfig InitConfig;
	};
}

