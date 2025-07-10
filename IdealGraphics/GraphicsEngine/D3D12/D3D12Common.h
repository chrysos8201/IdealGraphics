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
			eStandAlone, // 리소스를 혼자 사용할 경우
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

		// 리소스 전략 설정 by gyu
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

