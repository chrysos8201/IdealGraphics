#pragma once
#include "Core/Core.h"
#include "d3d12.h"

extern void VerifyD3D12Result(HRESULT D3DResult, const ANSICHAR* Code, const ANSICHAR* FileName, uint32 Line, ComPtr<ID3D12Device> Device);

#define VERIFYD3D12RESULT_EX(x, Device)	{HRESULT hres = x; if (FAILED(hres)) { VerifyD3D12Result(hres, #x, __FILE__, __LINE__, Device); }}
#define VERIFYD3D12RESULT(x)			{HRESULT hres = x; if (FAILED(hres)) { VerifyD3D12Result(hres, #x, __FILE__, __LINE__, nullptr); }}

inline bool IsGPUOnly(D3D12_HEAP_TYPE HeapType, const D3D12_HEAP_PROPERTIES* pCustomHeapProperties = nullptr)
{
	Check(HeapType == D3D12_HEAP_TYPE_CUSTOM ? pCustomHeapProperties != nullptr : true);
	return HeapType == D3D12_HEAP_TYPE_DEFAULT ||
		(HeapType == D3D12_HEAP_TYPE_CUSTOM &&
			(pCustomHeapProperties->CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE));
}

inline bool IsCPUAccessible(D3D12_HEAP_TYPE HeapType, const D3D12_HEAP_PROPERTIES* pCustomHeapProperties = nullptr)
{
	return !IsGPUOnly(HeapType, pCustomHeapProperties);
}


template <typename T>
constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}


// Val�� �־����� Alignment�� ��� �� ���� ����� ���� ������ �����Ѵ�.
// Alignment�� �ݵ�� 2�� �ŵ������̾�� ��.
// ex. 23, 16�̸� 16�� ��ȯ
template <typename T>
constexpr T AlignDown(T Val, uint64 Alignment)
{
	return (T)(((uint64)Val) & ~(Alignment - 1));
}

template <typename T>
constexpr bool IsAligned(T Val, uint64 Alignment)
{
	return !((uint64)Val & (Alignment - 1));
}
