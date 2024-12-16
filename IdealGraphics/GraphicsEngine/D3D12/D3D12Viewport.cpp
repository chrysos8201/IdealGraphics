#include "Core/Core.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
#include "GraphicsEngine/D3D12/D3D12Viewport.h"
using namespace Ideal;

Ideal::D3D12Viewport::D3D12Viewport(uint32 Width, uint32 Height)
	: m_width(Width),
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
		0,
		0,
		static_cast<LONG>(m_width),
		static_cast<LONG>(m_height)
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

void D3D12Viewport::ReSize(uint32 Width, uint32 Height)
{
	m_width = Width;
	m_height = Height;
	Init();
}

void D3D12Viewport::UpdatePostViewAndScissor(uint32 Width, uint32 Height)
{
	float viewWidthRatio = static_cast<float>(Width) / m_width;
	float viewHeightRatio = static_cast<float>(Height) / m_height;
	
	float x = 1.0f;
	float y = 1.0f;
	
	if (viewWidthRatio < viewHeightRatio)
	{
		// The scaled image's height will fit to the viewport's height and 
		// its width will be smaller than the viewport's width.
		x = viewWidthRatio / viewHeightRatio;
	}
	else
	{
		// The scaled image's width will fit to the viewport's width and 
		// its height may be smaller than the viewport's height.
		y = viewHeightRatio / viewWidthRatio;
	}
	
	m_viewport.TopLeftX = m_width * (1.0f - x) / 2.0f;
	m_viewport.TopLeftY = m_height * (1.0f - y) / 2.0f;
	m_viewport.Width = x * m_width;
	m_viewport.Height = y * m_height;
	
	m_scissorRect.left = static_cast<LONG>(m_viewport.TopLeftX);
	m_scissorRect.right = static_cast<LONG>(m_viewport.TopLeftX + m_viewport.Width);
	m_scissorRect.top = static_cast<LONG>(m_viewport.TopLeftY);
	m_scissorRect.bottom = static_cast<LONG>(m_viewport.TopLeftY + m_viewport.Height);
}
