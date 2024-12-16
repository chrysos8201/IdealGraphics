#pragma once
#include "Core/Core.h"

struct ID3D12Device;

namespace Ideal
{
	class D3D12ConstantBufferPool;
	struct ConstantBufferContainer;
}

namespace Ideal
{
	class D3D12DynamicConstantBufferAllocator
	{
	public:
		D3D12DynamicConstantBufferAllocator();
		virtual ~D3D12DynamicConstantBufferAllocator();

	public:
		void Init(const uint32& MaxCBVNum);
		std::shared_ptr<Ideal::ConstantBufferContainer> Allocate(ComPtr<ID3D12Device> Device, const uint32& Size);
		void Reset();

	private:
		std::map <uint32, std::shared_ptr<Ideal::D3D12ConstantBufferPool>> m_cbPools;
		uint32 m_maxCBVNumPerPool;
	};
}
