#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/ConstantBufferInfo.h"

struct ID3D12Device;
struct ID3D12RootSignature;
struct ID3D12GraphicsCommandList;
struct ID3D12PipelineState;

namespace Ideal
{
	class D3D12DescriptorHeap;
	class D3D12DynamicConstantBufferAllocator;
}
namespace Ideal
{
	class DebugLine
	{
	public:
		void Init(Vector3 p0, Vector3 p1);
		void DrawDebugLine(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool);

	private:
		Vector3 m_p0;
		Vector3 m_p1;
	};
}
