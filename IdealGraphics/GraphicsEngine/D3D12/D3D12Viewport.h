#pragma once
#include <d3dx12.h>
struct IDXGISwapChain3;

namespace Ideal
{
	class D3D12Viewport
	{
	public:
		D3D12Viewport(uint32 Width, uint32 Height);
		virtual ~D3D12Viewport();

		void Init();
		void ReSize(uint32 Width, uint32 Height);
		void UpdatePostViewAndScissor(uint32 Width, uint32 Height);

	public:
		const CD3DX12_VIEWPORT& GetViewport() const;
		const CD3DX12_RECT& GetScissorRect() const;

	private:
		uint32 m_width;
		uint32 m_height;

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
	};
}
