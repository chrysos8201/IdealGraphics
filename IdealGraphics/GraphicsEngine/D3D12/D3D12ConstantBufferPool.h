#pragma once
#include "Core/Core.h"
#include <d3d12.h>
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

namespace Ideal
{
	struct ConstantBufferContainer
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle;
		D3D12_GPU_VIRTUAL_ADDRESS GpuMemAddr;
		uint8* SystemMemAddr;
	};

	//-------------------------------------------------//
	// [				D3D12 Resource				 ] //
	// [CBV]  [CBV]  [CBV]  [CBV]  [CBV]  [CBV]  [CBV] //
	//-------------------------------------------------//

	class D3D12ConstantBufferPool
	{
	public:
		D3D12ConstantBufferPool();
		virtual ~D3D12ConstantBufferPool();

	public:
		void Init(ID3D12Device* Device, uint32 SizePerCBV, uint32 MaxCBVNum);
		std::shared_ptr<ConstantBufferContainer> Allocate();
		void Reset();
	private:
		ComPtr<ID3D12Resource> m_resource;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;

		// 시스템 메모리의 주소
		uint8* m_systemMemAddr;
		// 최대 할당할 수 있는 개수
		uint32 m_maxCBVNum;
		uint32 m_sizePerCBV;
		// 현재 할당받은 개수
		uint32 m_allocatedCBVNum;

		std::vector<std::shared_ptr<ConstantBufferContainer>> m_cbContainer;
	};
}
