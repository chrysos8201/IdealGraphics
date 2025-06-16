#pragma once
#include "Core\Core.h"
#include "RHI\RHIPoolAllocationData.h"
#include "D3D12Common.h"
#include "d3d12.h"

struct ID3D12Device;

namespace Ideal
{
	class RHIMemoryPool
	{
	public:
		RHIMemoryPool(uint64 InPoolSize, uint32 InPoolAlignment);
		
		virtual void Init();

	protected:
		const uint64 PoolSize;
		const uint32 PoolAlignment;	// 정렬 // 64KB 64MB 4kb

		RHIPoolAllocationData HeadBlock;
		std::vector<RHIPoolAllocationData*> FreeBlocks;
	};

	enum class EResourceAllocationStrategy
	{
		// ID3D12Heap 내부에서 CreatePlaced로 ID3D12Resource를 할당
		// CreatePlaced로 빠르다는 장점 있지만 ID3D12Resource 정렬을 따라야함..(64KB)
		PlacedResource,
		// 큰 ID3D12Resource를 만들어서 내부에서 수동으로 할당
		// 하나의 ID3D12Resource만 메모리 정렬을 맞추면 된다. 내부에서는 정렬 필요x
		ManualSubAllocation
	};

	struct D3D12ResourceInitConfig
	{
		D3D12_HEAP_TYPE HeapType;
		D3D12_HEAP_FLAGS HeapFlags;
		D3D12_RESOURCE_FLAGS ResourceFlags;
		D3D12_RESOURCE_STATES InitialResourceState;
	};

	class D3D12MemoryPool : public RHIMemoryPool, public D3D12DeviceChild
	{
	public:
		D3D12MemoryPool(ComPtr<ID3D12Device> InDevice, uint64 InPoolSize, uint32 InPoolAlignment, EResourceAllocationStrategy InAllocationStrategy, D3D12ResourceInitConfig InInitConfig);

		virtual void Init() override;

	private:
		EResourceAllocationStrategy AllocationStrategy;
		D3D12ResourceInitConfig InitConfig;

		ComPtr<ID3D12Resource> BackingResource;
		ComPtr<ID3D12Heap> BackingHeap;
	};
}