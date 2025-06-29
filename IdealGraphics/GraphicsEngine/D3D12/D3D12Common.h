#pragma once
#include "Core/Core.h"
#include "RHI/RHIPoolAllocationData.h"
#include "d3d12.h"

struct ID3D12Device;
using namespace Ideal;
namespace Ideal { class D3D12PoolAllocator; }

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
		D3D12DeviceChild(ID3D12Device* InDevice) 
			: Device(InDevice)
		{}

		inline ID3D12Device* GetDevice() const
		{
			Check(Device != nullptr);
			return Device;
		}

		void SetDevice(ID3D12Device* InDevice)
		{
			Device = InDevice;
		}

	protected:
		ID3D12Device* Device;
	};


	class D3D12ResourceLocation : public D3D12DeviceChild
	{
		friend class D3D12PoolAllocator;
	public:
		enum class ResourceLocationType : uint8 
		{
			eUndefined,
			eSubAllocation,
		};
	public:
		D3D12ResourceLocation(ID3D12Device* InDevice);

		void Clear();

		ID3D12Resource* GetResource() const { return UnderlyingResource; }
		uint64 GetOffsetFromBaseOfResource() const { return OffsetFromBaseOfResource; }
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return GPUVirtualAddress; }
		uint64 GetSize() const { return Size; }

		void SetType(ResourceLocationType Value) { Type = Value; }
		void SetResource(ID3D12Resource* Value);

		void SetPoolAllocator(D3D12PoolAllocator* Value) { PoolAllocator = Value; }
		D3D12PoolAllocator* GetPoolAllocator() { return PoolAllocator; }
		
		void SetMappedBaseAddress(void* Value) { MappedBaseAddress = Value; }
		void SetGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS Value) { GPUVirtualAddress = Value; }
		void SetOffsetFromBaseOfResource(uint64 Value) { OffsetFromBaseOfResource = Value; }
		void SetSize(uint64 Value) { Size = Value; }

		RHIPoolAllocationData& GetPoolAllocatorData() { return PoolData; }
	
	private:

		template<bool bReleaseResource>
		void InternalClear();

		ID3D12Resource* UnderlyingResource;

		D3D12PoolAllocator* PoolAllocator;
		RHIPoolAllocationData PoolData;
		ResourceLocationType Type;
		uint64 Size;

		void* MappedBaseAddress;
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress{};
		uint64 OffsetFromBaseOfResource;
	};
}

