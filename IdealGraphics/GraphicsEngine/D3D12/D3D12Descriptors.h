#pragma once
#include "Core/Core.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	class D3D12DescriptorHeap2;
}

namespace Ideal
{
	struct DescriptorAllocatorRange
	{
		DescriptorAllocatorRange(uint32 InFirst, uint32 InLast) : First(InFirst), Last(InLast) {}
		uint32 First;
		uint32 Last;
	};

	class D3D12DescriptorHandle2
	{
		friend D3D12DescriptorHeap2;

	public:
		D3D12DescriptorHandle2();
		D3D12DescriptorHandle2(CD3DX12_CPU_DESCRIPTOR_HANDLE InCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE InGPUHandle, uint32 InIndex, uint32 InNumDescriptors, uint32 InDescriptorHandleSize, bool ShaderVisible, std::shared_ptr<D3D12DescriptorHeap2> InOwnerHeap);
		~D3D12DescriptorHandle2();

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleStart() const
		{
			if (IsValid())
			{
				return CPUHandle;
			}
			else
			{
				return D3D12_CPU_DESCRIPTOR_HANDLE({ MAX_uint64 });
				//return D3D12_CPU_DESCRIPTOR_HANDLE();
			}
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleStart() const
		{
			if (IsValid())
			{
				return GPUHandle;
			}
			else
			{
				return D3D12_GPU_DESCRIPTOR_HANDLE({ MAX_uint64 });
				//return D3D12_GPU_DESCRIPTOR_HANDLE();
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleByIndex(uint32 Index) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleByIndex(uint32 Index) const;

	public:
		void Free();
		inline bool IsValid() const { return bValid; }

		void operator+=(uint32 IncrementSize);

	private:
		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
		uint32 Slot;
		uint32 NumDescriptors;
		uint32 DescriptorHandleSize;
		std::weak_ptr<D3D12DescriptorHeap2> OwnerHeap;
		bool bShaderVisible;
		bool bValid;
	};

	class D3D12DescriptorHeap2 : public std::enable_shared_from_this<D3D12DescriptorHeap2>
	{
		friend D3D12DescriptorHandle2;

	public:
		D3D12DescriptorHeap2(ComPtr<ID3D12Device> Device, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType, D3D12_DESCRIPTOR_HEAP_FLAGS InFlags, uint32 InxNumDescriptors);
		D3D12DescriptorHandle2 Allocate(uint32 NumDescriptors);
		void Free(D3D12DescriptorHandle2& Handle);

		void Destroy();

	private:
		void InitAllocator();

		/// <summary>
		/// 2025.02.20
		/// Free Range 목록에서 요청한 연속된 슬롯을 찾는 함수
		/// </summary>
		/// <param name="NumDescriptors"></param>
		/// <param name="OutSlot"></param>
		bool Allocate(uint32 NumDescriptors, uint32& OutSlot);

		/// <summary>
		/// 2025.02.20
		/// 할당받을 때 Slot을 기준으로 Descriptor를 해제한다. 
		/// </summary>
		/// <param name="Slot"></param>
		/// <param name="NumDescriptors"></param>
		void Free(uint32 Slot, uint32 NumDescriptors);

	private:
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;

		//uint32 NumDescriptors = 0;
		uint32 MaxNumDescriptors = 0;
		uint32 DescriptorHandleSize = 0;
		bool bShaderVisible = false;

		//CIndexCreator IndexCreator;

		// AllocatorRange
		std::vector<DescriptorAllocatorRange> Ranges;
	};
}