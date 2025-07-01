#pragma once
#include "Core\Core.h"
#include "RHI\RHIPoolAllocationData.h"
#include "d3d12.h"
#include "D3D12\D3D12Common.h"

struct ID3D12Device;
using namespace Ideal;

namespace Ideal
{
	class RHIMemoryPool
	{
	public:
		static uint32 GetAlignedSize(uint32 InSizeInBytes, uint32 InPoolAlignment, uint32 InAllocationAlignment);
		static uint32 GetAlignedOffset(uint32 InOffset, uint32 InPoolAlignment, uint32 InAllocationAlignment);
	
	public:
		RHIMemoryPool(uint16 InPoolIndex, uint64 InPoolSize, uint32 InPoolAlignment);
		
		virtual void Init();

		bool TryAllocate(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& AllocationData);
		void Deallocate(RHIPoolAllocationData& AllocationData);

		uint64 GetUsedSize() const { return (PoolSize - FreeSize); }
		bool IsFull() const { return FreeSize == 0; }

	protected:
		// Free Block 관리 함수
		int32 FindFreeBlock(uint32 InSizeInBytes, uint32 InAllocationAlignment) const;
		RHIPoolAllocationData* AddToFreeBlocks(RHIPoolAllocationData* InFreeBlock);
		void RemoveFromFreeBlocks(RHIPoolAllocationData* InFreeBlock);
		void ReleaseAllocationData(RHIPoolAllocationData* InData);

		RHIPoolAllocationData* GetNewAllocationData();

		void Validate();

		int16 PoolIndex;
		const uint64 PoolSize;
		const uint32 PoolAlignment;	// 정렬 // 64KB 64MB 4kb

		uint64 FreeSize;
		uint64 AlignmentWaste;
		uint64 AllocatedBlocks;

		RHIPoolAllocationData HeadBlock;
		std::vector<RHIPoolAllocationData*> FreeBlocks;
		std::vector<RHIPoolAllocationData*> AllocationDataPool;
	};

	class D3D12MemoryPool : public RHIMemoryPool, public D3D12DeviceChild
	{
	public:
		D3D12MemoryPool(ID3D12Device* InDevice, uint32 InPoolIndex, uint64 InPoolSize, uint32 InPoolAlignment, EResourceAllocationStrategy InAllocationStrategy, D3D12ResourceInitConfig InInitConfig);

		virtual void Init() override;

		ID3D12Resource* GetBackingResource() { return BackingResource.Get(); }
		ID3D12Heap* GetBackingHeap() { return BackingHeap.Get(); }
	private:
		EResourceAllocationStrategy AllocationStrategy;
		D3D12ResourceInitConfig InitConfig;
		uint32 PoolIndex;

		ComPtr<ID3D12Resource> BackingResource;
		ComPtr<ID3D12Heap> BackingHeap;
	};
}