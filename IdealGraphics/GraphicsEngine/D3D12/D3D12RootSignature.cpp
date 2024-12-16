//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "D3D12RootSignature.h"

Ideal::D3D12RootSignature::D3D12RootSignature()
{

}

Ideal::D3D12RootSignature::~D3D12RootSignature()
{

}

void Ideal::D3D12RootSignature::Create(uint32 NumRootParams, uint32 NumStaticSamplers)
{
	m_rootParameters.resize(NumRootParams);
	// TODO : sampler
}
