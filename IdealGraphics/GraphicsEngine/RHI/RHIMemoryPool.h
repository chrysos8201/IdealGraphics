#pragma once
#include "Core\Core.h"
#include "RHI\RHIPoolAllocationData.h"
#include "d3d12.h"
#include "D3D12\D3D12Common.h"

struct ID3D12Device;

namespace Ideal
{
	class RHIMemoryPool
	{
	public:
		static uint32 GetAlignedSize(uint32 InSizeInBytes, uint32 InPoolAlignment, uint32 InAllocationAlignment);

	public:
		RHIMemoryPool(uint64 InPoolSize, uint32 InPoolAlignment);
		
		virtual void Init();

		bool TryAllocate(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& AllocationData);

		bool IsFull() const { return FreeSize == 0; }

	protected:
		const uint64 PoolSize;
		const uint32 PoolAlignment;	// Á¤·Ä // 64KB 64MB 4kb

		uint64 FreeSize;

		RHIPoolAllocationData HeadBlock;
		std::vector<RHIPoolAllocationData*> FreeBlocks;
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