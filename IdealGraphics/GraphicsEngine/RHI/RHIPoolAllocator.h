#pragma once
#include "Core\Core.h"
#include "D3D12\D3D12Common.h"
#include "d3d12.h"
using namespace Ideal;

namespace Ideal { namespace Visualization { class RHIProfiler; } }
namespace Ideal { struct RHIPoolAllocationData; }

namespace Ideal { class RHIMemoryPool; }

struct ID3D12Device;

namespace Ideal
{
	class RHIPoolResource {};

	class RHIPoolAllocator
	{
	public:
		friend class Visualization::RHIProfiler;
	public:
		RHIPoolAllocator(uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled);
		virtual ~RHIPoolAllocator();

		void Defrag(const RHIContext& Context, uint32 InMaxCopySize, uint32& CurrentCopySize);

	protected:
		bool TryAllocateInternal(uint32 InSizeInBytes, uint32 InAllocationAlignment, RHIPoolAllocationData& InAllocationData);
		void DeallocateInternal(RHIPoolAllocationData& AllocationData);

		void SortPools();

		virtual RHIMemoryPool* CreateNewPool(uint32 InPoolIndex, uint32 InMinimumAllocationSize) = 0;

		friend class RHIMemoryPool;
		virtual bool HandleDefragRequest(const RHIContext& Context, RHIPoolAllocationData* InSourceBlock, RHIPoolAllocationData& InTmpTargetBlock) = 0;

		std::vector<RHIMemoryPool*> Pools;

		const uint64 DefaultPoolSize;
		const uint32 PoolAlignment;
		const uint64 MaxAllocationSize;
		const bool bDefragEnabled;

		// Pool�� ����
		std::vector<uint32> PoolAllocationOrder;
	};

	// Constant Buffer�� Alignment�� 256U�̴�. �ٸ� ���ҽ��� �ּ� 64KB�� ���� ��...
	const uint32 D3D12ManualSubAllocationAlignment = 256U;

	struct D3D12VRAMCopyOperation
	{
		enum ECopyType
		{
			BufferRegion,
			Resource,
		};

		ID3D12Resource* SourceResource;
		D3D12_RESOURCE_STATES SourceResourceState;
		uint32 SourceOffset;
		ID3D12Resource* DestResource;
		D3D12_RESOURCE_STATES DestResourceState;
		uint32 DestOffset;
		uint32 Size;
		ECopyType CopyType;
	};

	struct D3D12HeapAndOffset
	{
		ID3D12Heap* Heap;
		uint64 Offset;
	};

	struct D3D12VRAMCopyOperation
	{
		enum ECopyType
		{
			BufferRegion,
			Resource,
		};

		ID3D12Resource* SourceResource;
		D3D12_RESOURCE_STATES SourceResourceState;
		uint32 SourceOffset;
		ID3D12Resource* DestResource;
		D3D12_RESOURCE_STATES DestResourceState;
		uint32 DestOffset;
		uint32 Size;
		ECopyType CopyType;
	};

	class D3D12PoolAllocator : public RHIPoolAllocator, public D3D12DeviceChild
	{
	public:
		static EResourceAllocationStrategy GetResourceAllocationStrategy(D3D12_RESOURCE_FLAGS InResourceFlags, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment);

	public:
		// �ݵ�� ������ ���۽� FenceValue�� ����Ѵ�. // by Gyu
		void BeginAndSetFenceValue(uint64 InFenceValue);

	private:
		uint64 FenceValue = 0;

	public:
		D3D12PoolAllocator(ID3D12Device* InDevice, const D3D12ResourceInitConfig& InInitConfig, EResourceAllocationStrategy InAllocationStrategy, uint64 InDefaultPoolSize, uint32 InPoolAlignment, uint32 InMaxAllocationSize, bool bInDefragEnabled);

		bool SupportsAllocation(D3D12_HEAP_TYPE InHeapType, D3D12_HEAP_FLAGS InHeapFlags, D3D12_RESOURCE_FLAGS InResourceFlags, D3D12_RESOURCE_STATES InInitialResourceState, ED3D12ResourceStateMode InResourceStateMode, uint32 Alignment) const;
		void AllocateDefaultResource(D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& InResourceDesc, const D3D12_RESOURCE_STATES InResourceCreateState, ED3D12ResourceStateMode InResourceStateMode, uint32 InAllocationAlignment, const D3D12_CLEAR_VALUE* InClearValue, Ideal::D3D12ResourceLocation& ResourceLocation);
		void DeallocateResource(D3D12ResourceLocation& ResourceLocation, bool bDefragFree = false);

		// FencedValue�� �Ϸ�� Fence�� Value�� ������ �ȴ�.
		void CleanUpAllocations(uint64 InFrameLag, bool bForceFree = false);

		EResourceAllocationStrategy GetAllocationStrategy() const { return AllocationStrategy; }
		ID3D12Resource* GetBackingResource(D3D12ResourceLocation& InResourceLocation) const;
		D3D12HeapAndOffset GetBackingHeapAndAllocationOffsetInBytes(D3D12ResourceLocation& InResourceLocation) const;
		D3D12HeapAndOffset GetBackingHeapAndAllocationOffsetInBytes(const RHIPoolAllocationData& InAllocationData) const;
		bool IsOwner(D3D12ResourceLocation& ResourceLocation) const { return ResourceLocation.GetPoolAllocator() == this; }

	protected:
		virtual RHIMemoryPool* CreateNewPool(uint32 InPoolIndex, uint32 InMinimumAllocationSize) override;
		ID3D12Resource* CreatePlacedResource(const RHIPoolAllocationData& InAllocationData, const D3D12_RESOURCE_DESC& InDesc, D3D12_RESOURCE_STATES InCreateState, ED3D12ResourceStateMode InResourceStateMode, const D3D12_CLEAR_VALUE* InClearValue);

		virtual bool HandleDefragRequest(const RHIContext& Context, RHIPoolAllocationData* InSourceBlock, RHIPoolAllocationData& InTmpTargetBlock) override;

		struct FrameFencedAllocationData
		{
			enum class EOperation
			{
				Invalid,
				Deallocate,
				Unlock,
				Nop,
			};
			EOperation Operation = EOperation::Invalid;
			ID3D12Resource* PlacedResource = nullptr;
			uint64 FrameFence = 0;
			RHIPoolAllocationData* AllocationData = nullptr;
		};

		EResourceAllocationStrategy AllocationStrategy;
		D3D12ResourceInitConfig InitConfig;

		std::vector<FrameFencedAllocationData> FrameFencedOperations;
		uint64 PendingDeleteRequestSize = 0;	// FrameFencedOperationsť�� ĳ����, ���� ���� ��� �޸� ���� ��û�� �� ũ�⸦ ��Ÿ���� ����

		std::vector<D3D12VRAMCopyOperation> PendingCopyOps;

		std::vector<RHIPoolAllocationData*> AllocationDataPool;	// �޸� ���� ���� ������� ���Ҹ� ���� AllocationData Pool
	};
}

