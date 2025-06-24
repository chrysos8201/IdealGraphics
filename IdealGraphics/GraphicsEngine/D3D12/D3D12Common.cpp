#include "D3D12Common.h"

Ideal::D3D12ResourceLocation::D3D12ResourceLocation(ID3D12Device* InDevice)
	: D3D12DeviceChild(InDevice)
{
	Clear();
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
		// TODO :
		// ReleaseResource();
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


void Ideal::D3D12ResourceLocation::SetResource(ID3D12Resource* Value)
{
	Check(UnderlyingResource == nullptr);

	GPUVirtualAddress = Value->GetGPUVirtualAddress();

	UnderlyingResource = Value;
}
