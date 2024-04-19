#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"

namespace Ideal
{
	class D3D12Texture : public D3D12Resource
	{
	public:
		D3D12Texture();
		virtual ~D3D12Texture();

	public:
		void Create(
			ID3D12Device* Device,
			ID3D12GraphicsCommandList* CommandList,
			D3D12_CPU_DESCRIPTOR_HANDLE SRVHeapHandle,
			const std::wstring& Path
			);
	};
}

