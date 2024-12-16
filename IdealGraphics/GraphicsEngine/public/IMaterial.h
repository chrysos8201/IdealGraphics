#pragma once
#include <memory>

namespace Ideal
{
	class ITexture;
}

namespace Ideal
{
	class IMaterial
	{
	public:
		IMaterial() {}
		virtual ~IMaterial() {}

	public:
		virtual void SetBaseMap(std::shared_ptr<Ideal::ITexture> Texture) abstract;
		virtual void SetNormalMap(std::shared_ptr<Ideal::ITexture> Texture) abstract;
		// R : Metallic, A : Smoothness
		virtual void SetMaskMap(std::shared_ptr<Ideal::ITexture> Texture) abstract;

		virtual std::weak_ptr<Ideal::ITexture> GetBaseMap() abstract;
		virtual std::weak_ptr<Ideal::ITexture> GetNomralMap() abstract;
		virtual std::weak_ptr<Ideal::ITexture> GetMaskMap() abstract;

		virtual void SetTiling(float x, float y) abstract;
		virtual void SetOffset(float x, float y) abstract;

		virtual void SetAlphaClipping(bool IsAlphClipping) abstract;
		virtual void SetSurfaceTypeTransparent(bool IsTransparent) abstract;

		virtual void AddLayer(unsigned int LayerNum) abstract;
		virtual void DeleteLayer(unsigned int LayerNum) abstract;
		virtual void ChangeLayer(unsigned int LayerNum) abstract;
		virtual void ChangeLayerBitMask(unsigned int BitMask) abstract;
	};
}