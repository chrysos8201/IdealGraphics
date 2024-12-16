#pragma once
#include "ISprite.h"
#include "Core/Core.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/VertexInfo.h"
#include <memory>

struct ID3D12Device;
struct ID3D12CommandList;

namespace Ideal
{
	class D3D12Texture;
	class D3D12DynamicConstantBufferAllocator;
	class D3D12DynamicDescriptorHeap;
	class D3D12DescriptorHeap;
	template <typename> class IdealMesh;
	class ITexture;
}

namespace Ideal
{
	class IdealSprite : public ISprite
	{
	public:
		IdealSprite();
		virtual ~IdealSprite();

	public:
		// device, cb pool, commandlist 필요
		void DrawSprite(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> UIDescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, const Vector2& ScreenSize);
		// ComPtr<ID3D12Device> Device, ComPtr<ID3D12CommandList> CommandList, std::shared_ptr<Ideal::D3D12ConstantBufferPool> ConstantBufferPool
	public:
		//---Interface---//
		virtual void SetActive(bool IsActive) override;
		virtual bool GetActive() override;

		virtual Vector2 const& GetPosition() override { return m_cbSprite.Pos; }
		virtual float GetZ() override { return m_cbSprite.Z; }
		virtual float GetAlpha() override { return m_cbSprite.Alpha; }
		virtual Color const& GetColor() override { return m_cbSprite.SpriteColor; }

		virtual void SetSampleRect(const SpriteRectArea& Rect) override;
		virtual void SetPosition(const Vector2& Position) override;
		virtual void SetScale(const DirectX::SimpleMath::Vector2& Scale) override;
		virtual void SetZ(float Z) override;
		virtual void SetAlpha(float Alpha) override;
		virtual void SetColor(const DirectX::SimpleMath::Color& Color) override;

		virtual void SetTexture(std::weak_ptr<Ideal::ITexture> Texture) override;

		Vector2 const& GetSamplePosition() { return m_cbSprite.TexSamplePos; }
		Vector2 const& GetSampleSize() { return m_cbSprite.TexSampleSize; }

		void SetScreenSize(const Vector2& ScreenPos);
		void SetTextureSize(const Vector2& TextureSize);
		void SetTextureSamplePosition(const Vector2& TextureSamplePosition);
		void SetTextureSampleSize(const Vector2& TextureSampleSize);

		bool IsDirty() {return m_isDirty; }
		void SetDirty(bool Dirty) { m_isDirty = Dirty; }
	public:
		// default mesh를 넣어줄 것
		void SetMesh(std::shared_ptr<Ideal::IdealMesh<SimpleVertex>> Mesh);
		std::weak_ptr<Ideal::D3D12Texture> GetTexture() { return m_texture; }

		void CalculatePositionOffset();
		void ReSize(uint32 Width, uint32 Height);

	private:
		// 어차피 default mesh를 쓸건데 weak ptr이면 될 것 같음
		std::weak_ptr<Ideal::IdealMesh<SimpleVertex>> m_mesh;

		// 텍스쳐를 가지고 있는다...
		std::weak_ptr<Ideal::D3D12Texture> m_texture;
		// 스프라이트 정보!
		CB_Sprite m_cbSprite;
		bool m_isActive = true;
		bool m_isDirty = false;
	};
}
