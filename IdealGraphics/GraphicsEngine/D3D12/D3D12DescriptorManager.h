#pragma once
#include "Core/Types.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "Misc/Utils/IndexCreator.h"

#include <memory>
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	// 2024.06.23
	// 중첩 프레임에 사용되는 Descriptor Pool과 고정적으로 
	// 사용되는 CBV, SRV, UAV를 관리하는 Descriptor이 합쳐진 클래스
	//[----Pool----][-----Pool-----][----Fixed----]//

	// 2024.07.05
	// Fixed Pool에서 핸들 값이 반환되어야 한다.

	class D3D12DescriptorManager : public D3D12DescriptorHeapBase, public std::enable_shared_from_this<D3D12DescriptorManager>
	{
	public:
		D3D12DescriptorManager();
		virtual ~D3D12DescriptorManager();

		virtual EDescriptorType GetDescriptorType() override { return EDescriptorType::Static_DynamicHeap; }

	public:
		void Create(
			ComPtr<ID3D12Device> Device,
			uint32 NumDescriptorPool,
			uint32 MaxCountDescriptorPool,
			uint32 NumFixedDescriptorHeap,
			uint32 MaxCountFixedDescriptorHeap
		);

		void ResetPool(uint32 PoolIndex);
		//쉐이더 테이블을 만들 때 사용하는 Descriptor Heap일 경우 데이터를 해제 할 경우 어차피 다시 만들어야하니 그냥 Reset을 해도 될 듯?
		void ResetFixed(uint32 FixedDescriptorHeapIndex);

		Ideal::D3D12DescriptorHandle Allocate(uint32 DescriptorPoolIndex);
		Ideal::D3D12DescriptorHandle AllocateFixed(uint32 FixedDescriptorHeapIndex);

		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descriptorHeap; }

		void Free(int32 FixedDescriptorHeapIndex, int32 AllocatedIndex);

	private:
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
		uint32 m_maxCountDescriptorPool;
		std::vector<uint32> m_countDescriptorPool;
		uint32 m_numDescriptorPool;

		uint32 m_maxCountFixedDescriptorPool;
		std::vector<uint32> m_countFixedDescriptorPool;
		uint32 m_numFixedDescriptorPool;
		std::vector<IndexCreator> m_indexCreator;

		uint32 m_descriptorSize;
	};
}
