#include "D3D12Common.h"
#include "D3D12Util.h"
#include "RHI/RHIPoolAllocator.h"
#include "D3D12/D3D12Resource.h"

Ideal::D3D12ResourceLocation::D3D12ResourceLocation(ID3D12Device* InDevice)
	: D3D12DeviceChild(InDevice)
{
	Clear();
}

Ideal::D3D12ResourceLocation::~D3D12ResourceLocation()
{
	// 일단 명시적으로 불러주게 함
	//ReleaseResource();
}

void Ideal::D3D12ResourceLocation::Clear()
{
	InternalClear<true>();
}

template void D3D12ResourceLocation::InternalClear<false>();
template void D3D12ResourceLocation::InternalClear<true>();

template<bool bReleaseResource>
void Ideal::D3D12ResourceLocation::InternalClear()
{
	if (bReleaseResource)
	{
		ReleaseResource();
	}

	// 데이터 리셋
	Type = ResourceLocationType::eUndefined;
	UnderlyingResource = nullptr;
	MappedBaseAddress = nullptr;
	GPUVirtualAddress = 0;
	Size = 0;
	OffsetFromBaseOfResource = 0;
	std::memset(&PoolData, NULL, sizeof(PoolData));
	PoolAllocator = nullptr;
}


void Ideal::D3D12ResourceLocation::ReleaseResource()
{
	switch (Type)
	{
		case ResourceLocationType::eUndefined:
			break;
		case ResourceLocationType::eStandAlone:
			UnderlyingResource->Release();
			break;
		case ResourceLocationType::eSubAllocation:
		{
			Check(PoolAllocator != nullptr);
			// TODO: DeallocateResource
			PoolAllocator->DeallocateResource(*this);
		}
		break;
		default:
			break;
	}
}

void Ideal::D3D12ResourceLocation::SetResource(ID3D12Resource* Value)
{
	Check(UnderlyingResource == nullptr);

	GPUVirtualAddress = Value->GetGPUVirtualAddress();

	UnderlyingResource = Value;
}

bool Ideal::D3D12ResourceLocation::OnAllocationMoved(const RHIContext& Context, RHIPoolAllocationData* InNewData, D3D12_RESOURCE_STATES& OutCreateState)
{
	RHIPoolAllocationData& AllocationData = GetPoolAllocatorData();
	Check(InNewData == &AllocationData);
	Check(AllocationData.IsAllocated()); // 할당되어 있어야 한다
	Check(AllocationData.GetSize() == Size); // 같은 사이즈여야 하고
	Check(Type == ResourceLocationType::eSubAllocation); // 풀에서 할당되었어야 한다.
	Check(GetMappedBaseAddress() == nullptr); // 오직 VRAM만 가능

	// 리소스와 새로운 allocator를 가져온다.
	ID3D12Resource* CurrentResource = GetResource();
	D3D12PoolAllocator* NewAllocator = GetPoolAllocator();

	if (NewAllocator->GetAllocationStrategy() == EResourceAllocationStrategy::ManualSubAllocation)
	{
		Check(GetAllocationStrategy() != EResourceAllocationStrategy::PlacedResource);
		D3D12_GPU_VIRTUAL_ADDRESS OldGPUAddress = GPUVirtualAddress;
		OffsetFromBaseOfResource = AllocationData.GetOffset();
		UnderlyingResource = NewAllocator->GetBackingResource(*this);
		GPUVirtualAddress = UnderlyingResource->GetGPUVirtualAddress() + OffsetFromBaseOfResource;
	}
	else
	{
		Check(GetAllocationStrategy() == EResourceAllocationStrategy::PlacedResource);
		Check(OffsetFromBaseOfResource == 0);

		D3D12HeapAndOffset HeapAndOffset = NewAllocator->GetBackingHeapAndAllocationOffsetInBytes(*this);

		//D3D12_RESOURCE_STATES CreateState = D3D12_RESOURCE_STATE_TBD;
		D3D12_RESOURCE_STATES CreateState = D3D12_RESOURCE_STATE_COMMON;
		ED3D12ResourceStateMode ResourceStateMode = ED3D12ResourceStateMode::Default;

		std::wstring Name = L"TEST_DefraggedMemory";
		ID3D12Resource* NewResource = nullptr;
		{
			ID3D12Device* ParentDevice;
			CurrentResource->GetDevice(IID_PPV_ARGS(&ParentDevice));
			Check(ParentDevice->CreatePlacedResource(HeapAndOffset.Heap, HeapAndOffset.Offset, &CurrentResource->GetDesc(), CreateState, nullptr, IID_PPV_ARGS(&NewResource)));
			UnderlyingResource = NewResource;
			GPUVirtualAddress = UnderlyingResource->GetGPUVirtualAddress() + OffsetFromBaseOfResource;

			OutCreateState = CreateState;
		}

	}
	
	// TODO!!!!!!!
	// 해당 프레임 내에서 GPU 데이터 복사가 즉시 예약되어 실행될 것이 보장한다고 하는데 나중에 확인 해봐야함
	// -> Copy Operator가 같이 실행되는지 확인할 것
	Owner.lock()->ResourceRenamed((D3D12Context&)Context);

	// 이제 밖으로 나가서 Copy Opperator 만들자. 07.08 TODO 

	return true;
}

void Ideal::D3D12ResourceLocation::AsStandAlone(ID3D12Resource* Resource, D3D12_HEAP_TYPE ResourceHeapType, uint64 InSize)
{
	SetType(D3D12ResourceLocation::ResourceLocationType::eStandAlone);
	SetResource(Resource);
	SetSize(InSize);

	if (IsCPUAccessible(ResourceHeapType))
	{
		// TODO : Resource Map 업데이트 해야함
		//D3D12_RANGE range = {0, Is}
	}
	SetGPUVirtualAddress(Resource->GetGPUVirtualAddress());
}
