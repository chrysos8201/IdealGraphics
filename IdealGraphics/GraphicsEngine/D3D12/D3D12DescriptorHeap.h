#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

class IdealRenderer;

namespace Ideal
{
	//--------------------------Descriptor Handle---------------------------//

	class D3D12DescriptorHandle
	{
	public:
		D3D12DescriptorHandle();
		virtual ~D3D12DescriptorHandle();

		D3D12DescriptorHandle(const D3D12DescriptorHandle& Other);
		D3D12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle);
	
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_gpuHandle; }

	public:
		// operators
		void operator=(const D3D12DescriptorHandle& Other)
		{
			m_cpuHandle = Other.m_cpuHandle;
			m_gpuHandle = Other.m_gpuHandle;
		}
	
		void operator+=(uint32 IncrementSize)
		{
			m_cpuHandle.ptr += IncrementSize;
			m_gpuHandle.ptr += IncrementSize;
		}
	private:
		//CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	};

	//--------------------------Descriptor Heap---------------------------//

	class D3D12DescriptorHeap
	{
	public:
		D3D12DescriptorHeap();
		virtual ~D3D12DescriptorHeap();

	public:
		void Create(std::shared_ptr<IdealRenderer> Renderer, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32 MaxCount);
		void Create(ID3D12Device* Device, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32 MaxCount);
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descriptorHeap; }

		// �޸𸮸� �Ҵ��� �ּҸ� �޾ƿ��� Count��ŭ Handle�� �̵���Ų��.
		Ideal::D3D12DescriptorHandle Allocate(uint32 Count = 1);

	private:
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

		// ������ descriptor�� �Ҵ��� �� �ִ� ����
		uint32 m_numFreeDescriptors;
		uint32 m_descriptorSize;

		// ���� �Ҵ��� �� �ִ� �ּҸ� ����Ű�� �ִ� �ڵ�
		Ideal::D3D12DescriptorHandle m_freeHandle;
	};
}

// TODO LIST : 
// DescriptorHeap�� Descriptor�� ���������� 
//