#pragma once
#include "Core/Core.h"
#include "RHI/RHIPoolAllocator.h"

struct ID3D12Device;

namespace Ideal
{

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
		Default,		// Resource Flag�� ���� �ٲ� default�� ����ϸ� ����ϴ� �ʿ��� �˾Ƽ� �ٲ�
		SingleState,	// ���ҽ� ���� ������ �ʿ����� ���� ��� ����Ѵ�. ���ҽ��� ó�� �ʱ�ȭ�� ���·� ��� ���� ���̴�(ex. vertex buffer, index buffer)
		MultiState		// ���ҽ��� ���� ������ �ʿ��� ��� ����Ѵ�. ��Ÿ�� �� ���ҽ��� ���°� ��� �ٲ� �� �ִ�.(CopyDest���� UnorderedAccess�� �ȴٴ���...)
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

