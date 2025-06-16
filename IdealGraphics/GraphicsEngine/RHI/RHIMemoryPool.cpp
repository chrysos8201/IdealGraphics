#include "RHIMemoryPool.h"
#include "d3d12.h"
#include "d3dx12_core.h"

//using namespace Ideal;

Ideal::RHIMemoryPool::RHIMemoryPool(uint64 InPoolSize, uint32 InPoolAlignment)
	: PoolSize(InPoolSize)
	, PoolAlignment(InPoolAlignment)
{

}

void Ideal::RHIMemoryPool::Init()
{

}


/// <summary>
///  D3D12MemoryPool
/// </summary>
/// <param name="Device"></param>

Ideal::D3D12MemoryPool::D3D12MemoryPool(
	ComPtr<ID3D12Device> Device
	, uint64 InPoolSize
	, uint32 InPoolAlignment
	, EResourceAllocationStrategy InAllocationStrategy
	, D3D12ResourceInitConfig InInitConfig
	)
	: RHIMemoryPool(InPoolSize, InPoolAlignment)
	, D3D12DeviceChild(Device)
	, AllocationStrategy(InAllocationStrategy)
	, InitConfig(InInitConfig)
{

}

void Ideal::D3D12MemoryPool::Init()
{
	if (PoolSize == 0)
	{
		return;
	}

	// ID3D12Heap을 만드는 경우
	if (AllocationStrategy == EResourceAllocationStrategy::PlacedResource)
	{
		// GPU Resource 메모리 정렬이 4KB 이거나 64KB이어야 한다
		Check(PoolAlignment == D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT || PoolAlignment == D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		
		CD3DX12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InitConfig.HeapType);
		
		D3D12_HEAP_DESC Desc = {};
		Desc.SizeInBytes = PoolSize;
		Desc.Properties = HeapProps;
		Desc.Alignment = 0;
		Desc.Flags = InitConfig.HeapFlags;

		// Create heap
		Device->CreateHeap(&Desc, IID_PPV_ARGS(BackingHeap.GetAddressOf()));
	}
	// ID3D12Resource를 만드는 경우
	else
	{
		CD3DX12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InitConfig.HeapType);
		CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(PoolSize);
		Device->CreateCommittedResource(
			&HeapProps,
			InitConfig.HeapFlags,
			&ResourceDesc,
			InitConfig.InitialResourceState,
			nullptr,
			IID_PPV_ARGS(BackingResource.GetAddressOf())
		);
	}

	RHIMemoryPool::Init();
}