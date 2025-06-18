#pragma once
#include "Core/Core.h"
#include "RHI/RHIPoolAllocator.h"

struct ID3D12Device;

namespace Ideal
{

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

		bool operator==(const D3D12ResourceInitConfig& InOther) const
		{
			return HeapType == InOther.HeapType &&
				HeapFlags == InOther.HeapFlags &&
				ResourceFlags == InOther.ResourceFlags &&
				InitialResourceState == InOther.InitialResourceState;
		}
		D3D12_HEAP_TYPE HeapType;
		D3D12_HEAP_FLAGS HeapFlags;
		D3D12_RESOURCE_FLAGS ResourceFlags;
		D3D12_RESOURCE_STATES InitialResourceState;
	};

	enum class ED3D12ResourceStateMode
	{
		Default,		// Resource Flag에 따라 바뀔때 default를 사용하면 사용하는 쪽에서 알아서 바꿈
		SingleState,	// 리소스 상태 추적이 필요하지 않을 경우 사용한다. 리소스는 처음 초기화한 상태로 계속 사용될 것이다(ex. vertex buffer, index buffer)
		MultiState		// 리소스의 상태 추적이 필요할 경우 사용한다. 런타임 중 리소스의 상태가 계속 바뀔 수 있다.(CopyDest에서 UnorderedAccess가 된다던가...)
	};

	class D3D12DeviceChild
	{
	public:
		D3D12DeviceChild(ComPtr<ID3D12Device> InDevice)
		{
			Device = InDevice;
		}

		inline ComPtr<ID3D12Device> GetDevice() const
		{
			Check(Device != nullptr);
			return Device;
		}

	protected:
		ComPtr<ID3D12Device> Device;
	};


	class D3D12ResourceLocation : public D3D12DeviceChild
	{
		friend class D3D12PoolAllocator;
	public:
		void Clear();

		RHIPoolAllocationData& GetPoolAllocatorData() { return PoolData; }
	private:
		RHIPoolAllocationData PoolData;
	};
}

