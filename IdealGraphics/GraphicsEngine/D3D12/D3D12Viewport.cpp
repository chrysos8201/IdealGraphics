#include "GraphicsEngine/D3D12/D3D12Viewport.h"

using namespace Ideal;

D3D12Viewport::D3D12Viewport(HWND Hwnd, uint32 Width, uint32 Height)
	: m_hwnd(Hwnd),
	m_width(Width),
	m_height(Height)
{

}

D3D12Viewport::~D3D12Viewport()
{

}

void D3D12Viewport::Init()
{
	m_viewport = CD3DX12_VIEWPORT(
		0.f,
		0.f,
		static_cast<float>(m_width),
		static_cast<float>(m_height)
	);

	m_scissorRect = CD3DX12_RECT(
		0.f,
		0.f,
		static_cast<float>(m_width),
		static_cast<float>(m_height)
	);
}

const CD3DX12_VIEWPORT& D3D12Viewport::GetViewport() const
{
	return m_viewport;
}

const CD3DX12_RECT& D3D12Viewport::GetScissorRect() const
{
	return m_scissorRect;
}
