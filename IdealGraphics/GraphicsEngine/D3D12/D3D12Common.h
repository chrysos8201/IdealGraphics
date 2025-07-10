#pragma once
#include "Core/Core.h"
#include "RHI/RHIPoolAllocationData.h"
#include "RHI/RHIPoolAllocator.h"
#include "RHI/RHIResource.h"

#include "d3d12.h"

struct ID3D12Device;
namespace Ideal { class D3D12GPUBuffer; }
namespace Ideal { class D3D12Resource; }
using namespace Ideal;
namespace Ideal { class D3D12PoolAllocator; }

namespace Ideal
{
	class D3D12Context : public Ideal::RHIContext
	{
	public:
		ID3D12Device* Device;
		ID3D12CommandList* CommandList;

		inline void CreateSRV(const D3D12ResourceLocation& InResourceLocation, D3D12_CPU_DESCRIPTOR_HANDLE Handle, uint32 InNumElements, uint32 InElementSize) const
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Buffer.FirstElement = InResourceLocation.GetOffsetFromBaseOfResource() / InElementSize;
			SRVDesc.Buffer.NumElements = InNumElements;
			SRVDesc.Buffer.StructureByteStride = InElementSize;
			SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
			SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			Device->CreateShaderResourceView(InResourceLocation.GetResource(), &SRVDesc, Handle);
		}
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


	class D3D12ResourceLocation : public RHIPoolResource, public D3D12DeviceChild
	{
		friend class D3D12PoolAllocator;
	public:
		enum class ResourceLocationType : uint8 
		{
			eUndefined,
			eStandAlone, // ���ҽ��� ȥ�� ����� ���
			eSubAllocation,
		};
	public:
		D3D12ResourceLocation(ID3D12Device* InDevice);
		~D3D12ResourceLocation();

		void Clear();

		void SetOwner(std::shared_ptr<D3D12GPUBuffer> InOwner) { Owner = InOwner; }
		std::weak_ptr<D3D12GPUBuffer> GetOwner() const { return Owner; }

		ID3D12Resource* GetResource() const { return UnderlyingResource; }
		uint64 GetOffsetFromBaseOfResource() const { return OffsetFromBaseOfResource; }
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return GPUVirtualAddress; }
		uint64 GetSize() const { return Size; }

		void SetType(ResourceLocationType Value) { Type = Value; }
		void SetResource(ID3D12Resource* Value);

		void SetPoolAllocator(D3D12PoolAllocator* Value) { PoolAllocator = Value; }
		D3D12PoolAllocator* GetPoolAllocator() { return PoolAllocator; }
		
		void ClearAllocator() { PoolAllocator = nullptr; }

		void SetMappedBaseAddress(void* Value) { MappedBaseAddress = Value; }
		void SetGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS Value) { GPUVirtualAddress = Value; }
		void SetOffsetFromBaseOfResource(uint64 Value) { OffsetFromBaseOfResource = Value; }
		void SetSize(uint64 Value) { Size = Value; }

		RHIPoolAllocationData& GetPoolAllocatorData() { return PoolData; }
		void* GetMappedBaseAddress() const { return MappedBaseAddress; }

		bool OnAllocationMoved(const D3D12Context& Context, RHIPoolAllocationData* InNewData, D3D12_RESOURCE_STATES& OutCreateState);

		void AsStandAlone(ID3D12Resource* Resource, D3D12_HEAP_TYPE ResourceHeapType, uint64 InSize);

		// ���ҽ� ���� ���� by gyu
		void SetAllocationStrategy(EResourceAllocationStrategy Value) { PoolAllocationStrategy = Value; }
		EResourceAllocationStrategy GetAllocationStrategy() { return PoolAllocationStrategy; }

	private:

		template<bool bReleaseResource>
		void InternalClear();

		void ReleaseResource();

		std::weak_ptr<Ideal::D3D12GPUBuffer> Owner;
		ID3D12Resource* UnderlyingResource;

		D3D12PoolAllocator* PoolAllocator;
		RHIPoolAllocationData PoolData;
		ResourceLocationType Type;
		uint64 Size;

		EResourceAllocationStrategy PoolAllocationStrategy;

		void* MappedBaseAddress;
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress{};
		uint64 OffsetFromBaseOfResource;


		// GYU
	public:
		void SetResourceStateMode(ED3D12ResourceStateMode InResourceStateMode) { ResourceStateMode = InResourceStateMode; };
		ED3D12ResourceStateMode GetResourceStateMode() const { return ResourceStateMode; }

	private:
		ED3D12ResourceStateMode ResourceStateMode = ED3D12ResourceStateMode::Default;
	};
}

