#include "IdealSprite.h"
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include <d3d12.h>
#include <d3dx12.h>

Ideal::IdealSprite::IdealSprite()
{

}

Ideal::IdealSprite::~IdealSprite()
{

}

void Ideal::IdealSprite::DrawSprite(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> UIDescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, const Vector2& ScreenSize)
{
	SetScreenSize(ScreenSize);
	//m_cbSprite.ScreenSize = ScreenSize;

	//-------------DynamicTexture-------------//
	m_texture.lock()->UpdateTexture(Device, CommandList);

	// TODO :
	// Set Descriptor Heap
	// 
	// ConstantBuffer Pool
	// Copy Descriptor Simple
	// 
	// IASetVertexBuffer
	// IASetIndexBuffer
	// DrawIndexedInstanced
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_mesh.lock()->GetVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = m_mesh.lock()->GetIndexBufferView();

	CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	CommandList->IASetIndexBuffer(&indexBufferView);

	// Bind to Shader
	{
		// SRV
		if (!m_texture.expired())
		{
			auto handle = UIDescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), m_texture.lock()->GetSRV().GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetGraphicsRootDescriptorTable(Ideal::RectRootSignature::Slot::SRV_Sprite, handle.GetGpuHandle());
		}

		// Sprite Constant Buffer
		{
			auto cb = CBPool->Allocate(Device.Get(), sizeof(CB_Sprite));
			memcpy(cb->SystemMemAddr, &m_cbSprite, sizeof(CB_Sprite));
			auto handle = UIDescriptorHeap->Allocate();
			Device->CopyDescriptorsSimple(1, handle.GetCpuHandle(), cb->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommandList->SetGraphicsRootDescriptorTable(Ideal::RectRootSignature::Slot::CBV_RectInfo, handle.GetGpuHandle());
		}
	}
	// TODO : DrawIndexedInstanced
	CommandList->DrawIndexedInstanced(m_mesh.lock()->GetElementCount(), 1, 0, 0, 0);
}

void Ideal::IdealSprite::SetActive(bool IsActive)
{
	m_isActive = IsActive;
}

bool Ideal::IdealSprite::GetActive()
{
	return m_isActive;
}

void Ideal::IdealSprite::SetSampleRect(const SpriteRectArea& Rect)
{
	SetTextureSamplePosition(Vector2(Rect.x, Rect.y));
	SetTextureSampleSize(Vector2(Rect.width, Rect.height));
	//m_cbSprite.TexSamplePos = Vector2(Rect.left, Rect.top);
	//m_cbSprite.TexSampleSize = Vector2(Rect.right, Rect.bottom);
}

void Ideal::IdealSprite::SetScreenSize(const Vector2& ScreenPos)
{
	m_cbSprite.ScreenSize = ScreenPos;
}

void Ideal::IdealSprite::SetPosition(const Vector2& Position)
{
	m_cbSprite.Pos = Position;
	CalculatePositionOffset();
}

void Ideal::IdealSprite::SetScale(const DirectX::SimpleMath::Vector2& Scale)
{
	m_cbSprite.Scale = Scale;
}

void Ideal::IdealSprite::SetTextureSize(const Vector2& TextureSize)
{
	m_cbSprite.TexSize = TextureSize;
}

void Ideal::IdealSprite::SetTextureSamplePosition(const Vector2& TextureSamplePosition)
{
	m_cbSprite.TexSamplePos = TextureSamplePosition;
}

void Ideal::IdealSprite::SetTextureSampleSize(const Vector2& TextureSampleSize)
{
	m_cbSprite.TexSampleSize = TextureSampleSize;
}

void Ideal::IdealSprite::SetZ(float Z)
{
	m_cbSprite.Z = Z;
	SetDirty(true);
}

void Ideal::IdealSprite::SetAlpha(float Alpha)
{
	m_cbSprite.Alpha = Alpha;
}

void Ideal::IdealSprite::SetColor(const DirectX::SimpleMath::Color& Color)
{
	m_cbSprite.SpriteColor = Color;
}

void Ideal::IdealSprite::SetTexture(std::weak_ptr<Ideal::ITexture> Texture)
{
	m_texture = std::static_pointer_cast<Ideal::D3D12Texture>(Texture.lock());
	uint32 width = (uint32)m_texture.lock()->GetResource()->GetDesc().Width;
	uint32 height = (uint32)m_texture.lock()->GetResource()->GetDesc().Height;

	SetTextureSize(Vector2(width, height));
	// Default
	SetTextureSamplePosition(Vector2(0, 0));
	SetTextureSampleSize(Vector2(width, height));
}

void Ideal::IdealSprite::SetMesh(std::shared_ptr<Ideal::IdealMesh<SimpleVertex>> Mesh)
{
	m_mesh = Mesh;
}

void Ideal::IdealSprite::CalculatePositionOffset()
{
	m_cbSprite.PosOffset = m_cbSprite.Pos / m_cbSprite.ScreenSize;
	MyDebugConsoleMessage(
		"PosOffset : "
		+ std::to_string(m_cbSprite.PosOffset.x)
		+ " , "
		+ std::to_string(m_cbSprite.PosOffset.y)
		+ "\n"
		);
}

void Ideal::IdealSprite::ReSize(uint32 Width, uint32 Height)
{
	m_cbSprite.ScreenSize.x = Width;
	m_cbSprite.ScreenSize.y = Height;
	// CalculatePositionOffset();
	// 생각해보니 offset 계산을 Resize때 다시 안할려고 한건데 
	// 그래서 삭제. 
	// CalculatePositionOffset();
}