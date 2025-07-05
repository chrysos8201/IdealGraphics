#include "D3D12Common.h"
#include "D3D12Util.h"
#include "RHI/RHIPoolAllocator.h"

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
