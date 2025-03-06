#include "GraphicsEngine/D3D12/D3D12Descriptors.h"
#include "GraphicsEngine/D3D12/D3D12Util.h"

D3D12DescriptorHandle2::D3D12DescriptorHandle2(CD3DX12_CPU_DESCRIPTOR_HANDLE InCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE InGPUHandle, uint32 InIndex, uint32 InNumDescriptors, uint32 InDescriptorHandleSize, bool ShaderVisible, std::shared_ptr<D3D12DescriptorHeap2> InOwnerHeap)
	: CPUHandle(InCPUHandle)
	, GPUHandle(InGPUHandle)
	, Slot(InIndex)
	, NumDescriptors(InNumDescriptors)
	, DescriptorHandleSize(InDescriptorHandleSize)
	, OwnerHeap(InOwnerHeap)
	, bShaderVisible(ShaderVisible)
	, bValid(true)
{

}

D3D12DescriptorHandle2::D3D12DescriptorHandle2()
	: CPUHandle()
	, GPUHandle()
	, Slot(MAX_uint32)
	, NumDescriptors(0)
	, DescriptorHandleSize(0)
	, OwnerHeap()
	, bShaderVisible(false)
	, bValid(false)
{

}

D3D12DescriptorHandle2::~D3D12DescriptorHandle2() = default;

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHandle2::GetCPUDescriptorHandleByIndex(uint32 Index) const
{
	if (IsValid())
	{
		if (Index < NumDescriptors)
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(CPUHandle).Offset(Index * DescriptorHandleSize);
		}
		else
		{
			__debugbreak();
		}
	}
	return CD3DX12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHandle2::GetGPUDescriptorHandleByIndex(uint32 Index) const
{
	if (IsValid())
	{
		if (bShaderVisible)
		{
			if (Index < NumDescriptors)
			{
				return CD3DX12_GPU_DESCRIPTOR_HANDLE(GPUHandle).Offset(Index * DescriptorHandleSize);
			}
			else
			{
				__debugbreak();
			}
		}
	}
	return CD3DX12_GPU_DESCRIPTOR_HANDLE();
}

void D3D12DescriptorHandle2::Free()
{
	//OwnerHeap.lock()->Free(*this);
	if (!bValid)
	{
		OwnerHeap.lock()->Free(Slot, NumDescriptors);
		bValid = false;
	}
}

void D3D12DescriptorHandle2::operator+=(uint32 IncrementSize)
{
	CPUHandle.Offset(IncrementSize);
	if (bShaderVisible)
	{
		GPUHandle.Offset(IncrementSize);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CD3D12DescriptorHeap

D3D12DescriptorHeap2::D3D12DescriptorHeap2(ComPtr<ID3D12Device> Device, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType, D3D12_DESCRIPTOR_HEAP_FLAGS InFlags, uint32 InMaxNumDescriptors)
{
	if (InFlags == D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
		bShaderVisible = false;
	}

	MaxNumDescriptors = InMaxNumDescriptors;
	//IndexCreator.Init(MaxNumDescriptors);
	InitAllocator();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MaxNumDescriptors;
	heapDesc.Type = InHeapType;
	heapDesc.Flags = InFlags;
	VERIFYD3D12RESULT(Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(DescriptorHeap.GetAddressOf())));
	//Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(DescriptorHeap.GetAddressOf()));

	DescriptorHandleSize = Device->GetDescriptorHandleIncrementSize(heapDesc.Type);
}

D3D12DescriptorHandle2 D3D12DescriptorHeap2::Allocate(uint32 NumDescriptors)
{
	uint32 outSlot = 0;
	bool isValid = Allocate(NumDescriptors, outSlot);
	if (!isValid)
	{
		//return CD3D12DescriptorHandle();
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuAddress;
	gpuAddress.ptr = MAX_uint64;
	// GPU Address가 필요할 경우
	if (bShaderVisible)
	{
		gpuAddress = DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}

	D3D12DescriptorHandle2 retHandle = D3D12DescriptorHandle2(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeap->GetCPUDescriptorHandleForHeapStart()),
		gpuAddress,
		outSlot,
		NumDescriptors,
		DescriptorHandleSize,
		bShaderVisible,
		shared_from_this()
	);

	retHandle += outSlot * DescriptorHandleSize;

	retHandle.bValid = isValid;
	return retHandle;
}

void D3D12DescriptorHeap2::Free(D3D12DescriptorHandle2& Handle)
{
	//IndexCreator.Free(Handle.Slot);
	//Handle.bValid = false;

}

void D3D12DescriptorHeap2::InitAllocator()
{
	// 첫 범위를 크게 하나로 만든다.
	// |<-----------Range----------->|
	Ranges.push_back({ 0, MaxNumDescriptors - 1 });
}

bool D3D12DescriptorHeap2::Allocate(uint32 NumDescriptors, uint32& OutSlot)
{
	if (const uint32 NumRanges = (uint32)Ranges.size(); NumRanges > 0)
	{
		uint32 Index = 0;
		do
		{
			// 현재 범위
			DescriptorAllocatorRange& CurrentRange = Ranges[Index];
			// 각 Free Range는 [First, Last]로 표현되며, 범위의 크기는 Last - First + 1이다.
			const uint32 Size = CurrentRange.Last - CurrentRange.First + 1;
			if (NumDescriptors <= Size)
			{
				// 요청한 슬롯 수가 Free Range 내에 있는 경우
				uint32 First = CurrentRange.First;
				if (NumDescriptors == Size && Index + 1 < NumRanges)
				{
					Ranges.erase(Ranges.begin() + Index);
				}
				else
				{
					CurrentRange.First += NumDescriptors;
				}
				OutSlot = First;

				return true;
			}
			++Index;
		} while (Index < NumRanges);
	}

	// 실패할 경우
	OutSlot = MAX_uint32;
	return false;
}

void D3D12DescriptorHeap2::Free(uint32 Slot, uint32 NumDescriptors)
{
	// 현재 범위의 첫 위치
	const uint32 offset = Slot;
	if (offset == MAX_uint32 || NumDescriptors == 0)
	{
		// 유효하지 않은 경우
		return;
	}
	// 현재 범위의 마지막 위치
	const uint32 end = offset + NumDescriptors;
	// Range목록을 이진 탐색으로 적절한 위치 찾기
	uint32 index0 = 0;
	uint32 index1 = (uint32)Ranges.size() - 1;
	for (;;)
	{
		const uint32 index = (index0 + index1) / 2;

		//해제 범위가 현재 range의 앞쪽에 있음
		if (offset < Ranges[index].First)
		{
			if (end >= Ranges[index].First)
			{
				// 해제 범위와 현재 range가 인접하거나 겹치지 않고 딱 맞을 경우

				// 해제 
				if (index > index0 && offset - 1 == Ranges[index - 1].Last)
				{
					// 이전 Range와 인접하므로 합친다.
					// 이전 Range의 Last를 현재 Range의 Last로 바꾸고, 현재 Range를 제거한다.
					// [0,5][6,10] -> [0, 10]
					Ranges[index - 1].Last = Ranges[index].Last;
					Ranges.erase(Ranges.begin() + index);
				}
				else
				{
					// 현재 Range와만 병합
					Ranges[index].First = offset;
				}
				// 병합 후 함수 종료
				return;
			}
			else
			{
				// 해제 범위가 현재 Range보다 완전히 앞에 있으나 인접하지 않음
				if (index != index0)
				{
					// 이진 탐색
					// 상단(더 작은 인덱스) 범위로 좁힘
					index1 = index - 1;
				}
				else
				{
					// 올바른 위치에 새 Free Range 삽입
					Ranges.emplace(Ranges.begin() + index, offset, end - 1);
					return;
				}
			}
		}
		// 해제 범위가 현재 Range의 뒤쪽에 있음
		else if (offset > Ranges[index].Last)
		{
			// 현재 Range와 해제 범위가 인접하므로 병합
			if (offset - 1 == Ranges[index].Last)
			{
				if (index < index1 && end == Ranges[index + 1].First)
				{
					// 해제 범위가 다음 Range와도 인접하므로 세 범위를 병합

					//	[   index     ]                 [     index+1     ]
					//                [      Free       ]
					// 위 상황에서 아래로 병합
					//	[                     index                       ] 
					//
					Ranges[index].Last = Ranges[index + 1].Last;
					Ranges.erase(Ranges.begin() + index + 1);
				}
				else
				{
					// 현재 Range와 단순 병합

					//	[   index     ]                         [     index+1     ]
					//                [      Free       ]
					// 위 상황에서 아래로 병합
					//	[             index             ]       [     index+1     ]
					//
					Ranges[index].Last += NumDescriptors;
				}
				return;
			}
			// 해제 범위가 현재 Range의 뒤쪽이지만 인접하지 않음
			else
			{
				if (index != index1)
				{
					// 이진 탐색
					// -> 하단 범위로 좁힘
					//      <index>
					//	[   index0    ]                         [     index1     ]
					//                        [      Free       ]
					//								   or
					//                        [     Free     ]
					// 위 상황에서 아래로 
					//												  <index>
					//	[   index0    ]                         [     index1     ]
					//                        [      Free       ]
					//								   or
					//                        [     Free     ]
					index0 = index + 1;
				}
				else
				{
					Ranges.emplace(Ranges.begin() + index + 1, offset, end - 1);
					return;
				}
			}
		}
		else
		{
			// 해제 범위가 이미 free range 내부에 있음
			// 중복 해제 시도이므로 브레이크
			__debugbreak();
		}
	}
}