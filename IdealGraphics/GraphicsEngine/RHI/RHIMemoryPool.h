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
		const uint32 PoolAlignment;	// ���� // 64KB 64MB 4kb

		RHIPoolAllocationData HeadBlock;
		std::vector<RHIPoolAllocationData*> FreeBlocks;
	};

	enum class EResourceAllocationStrategy
	{
		// ID3D12Heap ���ο��� CreatePlaced�� ID3D12Resource�� �Ҵ�
		// CreatePlaced�� �����ٴ� ���� ������ ID3D12Resource ������ �������..(64KB)
		PlacedResource,
		// ū ID3D12Resource�� ���� ���ο��� �������� �Ҵ�
		// �ϳ��� ID3D12Resource�� �޸� ������ ���߸� �ȴ�. ���ο����� ���� �ʿ�x
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