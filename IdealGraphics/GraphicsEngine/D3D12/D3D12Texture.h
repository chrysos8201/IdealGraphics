#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"

class IdealRenderer;
namespace Ideal
{
	class D3D12Texture : public D3D12Resource
	{
	public:
		D3D12Texture();
		virtual ~D3D12Texture();

	public:
		void Create(
			std::shared_ptr<IdealRenderer> Renderer, const std::wstring& Path
			);

		// 2024.04.21 : 디스크립터가 할당된 위치를 가져온다.
		//D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() { return m_srvHandle.GetCpuHandle(); }
		Ideal::D3D12DescriptorHandle GetDescriptorHandle() { return m_srvHandle; }

	private:
		// 2024.04.21
		// Texture가 descriptor heap에 할당된 주소를 가지고 있는다.
		Ideal::D3D12DescriptorHandle m_srvHandle;
	};
}

