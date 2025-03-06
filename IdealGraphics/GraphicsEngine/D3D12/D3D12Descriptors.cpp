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
	// GPU Address�� �ʿ��� ���
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
	// ù ������ ũ�� �ϳ��� �����.
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
			// ���� ����
			DescriptorAllocatorRange& CurrentRange = Ranges[Index];
			// �� Free Range�� [First, Last]�� ǥ���Ǹ�, ������ ũ��� Last - First + 1�̴�.
			const uint32 Size = CurrentRange.Last - CurrentRange.First + 1;
			if (NumDescriptors <= Size)
			{
				// ��û�� ���� ���� Free Range ���� �ִ� ���
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

	// ������ ���
	OutSlot = MAX_uint32;
	return false;
}

void D3D12DescriptorHeap2::Free(uint32 Slot, uint32 NumDescriptors)
{
	// ���� ������ ù ��ġ
	const uint32 offset = Slot;
	if (offset == MAX_uint32 || NumDescriptors == 0)
	{
		// ��ȿ���� ���� ���
		return;
	}
	// ���� ������ ������ ��ġ
	const uint32 end = offset + NumDescriptors;
	// Range����� ���� Ž������ ������ ��ġ ã��
	uint32 index0 = 0;
	uint32 index1 = (uint32)Ranges.size() - 1;
	for (;;)
	{
		const uint32 index = (index0 + index1) / 2;

		//���� ������ ���� range�� ���ʿ� ����
		if (offset < Ranges[index].First)
		{
			if (end >= Ranges[index].First)
			{
				// ���� ������ ���� range�� �����ϰų� ��ġ�� �ʰ� �� ���� ���

				// ���� 
				if (index > index0 && offset - 1 == Ranges[index - 1].Last)
				{
					// ���� Range�� �����ϹǷ� ��ģ��.
					// ���� Range�� Last�� ���� Range�� Last�� �ٲٰ�, ���� Range�� �����Ѵ�.
					// [0,5][6,10] -> [0, 10]
					Ranges[index - 1].Last = Ranges[index].Last;
					Ranges.erase(Ranges.begin() + index);
				}
				else
				{
					// ���� Range�͸� ����
					Ranges[index].First = offset;
				}
				// ���� �� �Լ� ����
				return;
			}
			else
			{
				// ���� ������ ���� Range���� ������ �տ� ������ �������� ����
				if (index != index0)
				{
					// ���� Ž��
					// ���(�� ���� �ε���) ������ ����
					index1 = index - 1;
				}
				else
				{
					// �ùٸ� ��ġ�� �� Free Range ����
					Ranges.emplace(Ranges.begin() + index, offset, end - 1);
					return;
				}
			}
		}
		// ���� ������ ���� Range�� ���ʿ� ����
		else if (offset > Ranges[index].Last)
		{
			// ���� Range�� ���� ������ �����ϹǷ� ����
			if (offset - 1 == Ranges[index].Last)
			{
				if (index < index1 && end == Ranges[index + 1].First)
				{
					// ���� ������ ���� Range�͵� �����ϹǷ� �� ������ ����

					//	[   index     ]                 [     index+1     ]
					//                [      Free       ]
					// �� ��Ȳ���� �Ʒ��� ����
					//	[                     index                       ] 
					//
					Ranges[index].Last = Ranges[index + 1].Last;
					Ranges.erase(Ranges.begin() + index + 1);
				}
				else
				{
					// ���� Range�� �ܼ� ����

					//	[   index     ]                         [     index+1     ]
					//                [      Free       ]
					// �� ��Ȳ���� �Ʒ��� ����
					//	[             index             ]       [     index+1     ]
					//
					Ranges[index].Last += NumDescriptors;
				}
				return;
			}
			// ���� ������ ���� Range�� ���������� �������� ����
			else
			{
				if (index != index1)
				{
					// ���� Ž��
					// -> �ϴ� ������ ����
					//      <index>
					//	[   index0    ]                         [     index1     ]
					//                        [      Free       ]
					//								   or
					//                        [     Free     ]
					// �� ��Ȳ���� �Ʒ��� 
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
			// ���� ������ �̹� free range ���ο� ����
			// �ߺ� ���� �õ��̹Ƿ� �극��ũ
			__debugbreak();
		}
	}
}