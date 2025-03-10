#pragma once
#include "IMaterial.h"
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "GraphicsEngine/ConstantBufferInfo.h"
#include "GraphicsEngine/D3D12/D3D12Descriptors.h"
#include "GraphicsEngine/D3D12/Raytracing/RayTracingFlagManger.h"
#include "GraphicsEngine/IdealLayer.h"

namespace Ideal
{
	class IdealRenderer;
	class D3D12Texture;
	class ResourceManager;
	class IdealStaticMeshObject;
	class IdealSkinnedMeshObject;
}

namespace Ideal
{
	class IdealMaterial : public ResourceBase, public IMaterial
	{
	public:
		IdealMaterial();
		virtual ~IdealMaterial();

	public:
		//--------IMaterial Interface--------//
		virtual void SetBaseMap(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void SetNormalMap(std::shared_ptr<Ideal::ITexture> Texture) override;
		// R : Metallic, A : Smoothness
		virtual void SetMaskMap(std::shared_ptr<Ideal::ITexture> Texture) override;

		virtual std::weak_ptr<Ideal::ITexture> GetBaseMap() override;
		virtual std::weak_ptr<Ideal::ITexture> GetNomralMap() override;
		virtual std::weak_ptr<Ideal::ITexture> GetMaskMap() override;

		virtual void SetTiling(float x, float y) override;
		virtual void SetOffset(float x, float y) override;

		virtual void SetAlphaClipping(bool IsAlphBlending) override;
		virtual void SetSurfaceTypeTransparent(bool IsTransparent) override;

	public:
		void SetAmbient(Color c) { m_ambient = c; }
		void SetDiffuse(Color c) { m_diffuse = c; }
		void SetSpecular(Color c) { m_specular = c; }
		void SetEmissive(Color c) { m_emissive = c; }

		void SetMetallicFactor(const float& f) { m_metallicFactor = f; }
		void SetRoughnessFactor(const float& f) { m_roughnessFactor = f; }

		void Create(std::shared_ptr<Ideal::ResourceManager> ResourceManager);
		// Ω¶¿Ã¥ı∂˚ πŸ¿ŒµÂ∏¶ «—¥Ÿ.
		void BindToShader(std::shared_ptr<Ideal::IdealRenderer> Renderer);

		void SetDiffuseTextureFile(std::wstring& File) { m_diffuseTextureFile = File; }
		void SetIsUseDiffuse(bool b) { m_cbMaterialInfo.bUseDiffuseMap = b; }
		void SetSpecularTextureFile(std::wstring& File) { m_specularTextureFile = File; }

		void SetEmissiveTextureFile(std::wstring& File) { m_emissiveTextureFile = File; }
		void SetNormalTextureFile(std::wstring& File) { m_normalTextureFile = File; }
		void SetIsUseNormal(bool b) { m_cbMaterialInfo.bUseNormalMap = b; }

		void SetMetallicTextureFile(std::wstring& File) { m_metallicTextureFile = File; }
		void SetIsUseMetallic(bool b) { m_cbMaterialInfo.bUseMetallicMap = b; }
		void SetRoughnessTextureFile(std::wstring& File) { m_roughnessTextureFile = File; }
		void SetIsUseRoughness(bool b) { m_cbMaterialInfo.bUseRoughnessMap = b; }

		//std::shared_ptr<Ideal::D3D12Texture> GetDiffuseTexture() { return m_diffuseTexture; }
		//std::shared_ptr<Ideal::D3D12Texture> GetSpecularTexture() { return m_specularTexture; }
		//std::shared_ptr<Ideal::D3D12Texture> GetEmissiveTexture() { return m_emissiveTexture; }
		//std::shared_ptr<Ideal::D3D12Texture> GetNormalTexture() { return m_normalTexture; }
		//
		//std::shared_ptr<Ideal::D3D12Texture> GetMetallicTexture() { return m_metalicTexture; }
		//std::shared_ptr<Ideal::D3D12Texture> GetRoughnessTexture() { return m_roughnessTexture; }
		//
		//std::shared_ptr<Ideal::D3D12Texture> GetMaskTexture() { return m_maskTexture; }


		CB_MaterialInfo const& GetMaterialInfo() { return m_cbMaterialInfo; }
		bool GetIsAlphaClipping() { return m_isAlphaClipping; }
		bool GetIsTransmissive() { return m_cbMaterialInfo.bIsTransmissive; }
	private:
		Vector2 m_Tiling;
		Vector2 m_Offset;

		Color m_ambient;
		Color m_diffuse;
		Color m_specular;
		Color m_emissive;

		float m_metallicFactor;
		float m_roughnessFactor;

		std::wstring m_diffuseTextureFile;
		std::wstring m_specularTextureFile;
		std::wstring m_emissiveTextureFile;
		std::wstring m_normalTextureFile;

		std::wstring m_metallicTextureFile;
		std::wstring m_roughnessTextureFile;

		std::weak_ptr<Ideal::D3D12Texture> m_diffuseTexture;
		std::shared_ptr<Ideal::D3D12Texture> m_specularTexture;
		std::shared_ptr<Ideal::D3D12Texture> m_emissiveTexture;
		std::weak_ptr<Ideal::D3D12Texture> m_normalTexture;

		std::weak_ptr<Ideal::D3D12Texture> m_metalicTexture;
		std::weak_ptr<Ideal::D3D12Texture> m_roughnessTexture;

		std::weak_ptr<Ideal::D3D12Texture> m_maskTexture;

	public:
		//--- Ray Tracing Info ---//
		void FreeInRayTracing();
		void CopyHandleToRayTracingDescriptorTable(ComPtr<ID3D12Device> Device);
		Ideal::D3D12DescriptorHandle2 GetBaseTextureHandleInRayTracing() { return m_diffuseTextureInRayTracing; }
		Ideal::D3D12DescriptorHandle2 GetNormalTextureHandleInRayTracing() { return m_normalTextureInRayTracing; }
		Ideal::D3D12DescriptorHandle2 GetMaskTextureHandleInRayTracing() { return m_maskTextureInRayTracing; }
		void SetDiffuseTextureHandleInRayTracing(Ideal::D3D12DescriptorHandle2 handle) { m_diffuseTextureInRayTracing = handle; }
		void SetNormalTextureHandleInRayTracing(Ideal::D3D12DescriptorHandle2 handle) { m_normalTextureInRayTracing = handle; }
		void SetMaskTextureHandleInRayTracing(Ideal::D3D12DescriptorHandle2 handle) { m_maskTextureInRayTracing = handle; }
		void AddRefCountInRayTracing() { m_refCountInRayTracing++; }

	private:
		// ray tracing descriptor table handles
		Ideal::D3D12DescriptorHandle2 m_diffuseTextureInRayTracing;
		Ideal::D3D12DescriptorHandle2 m_normalTextureInRayTracing;
		Ideal::D3D12DescriptorHandle2 m_maskTextureInRayTracing;
		uint64 m_refCountInRayTracing = 0;

	private:
		CB_Material m_cbMaterialData;
		CB_MaterialInfo	m_cbMaterialInfo;

	public:
		//void MaterialChanged(std::shared_ptr<Ideal::IdealMaterial> Material) { m_prevMaterial = Material; m_isMaterialChanged = true; }
		void TextureChanged() { m_isTextureChanged = true; Ideal::Singleton::RayTracingFlagManger::GetInstance().SetTextureChanged(); }
		bool IsTextureChanged() { return m_isTextureChanged; }

	private:
		//std::shared_ptr<Ideal::IdealMaterial> m_prevMaterial;
		//bool m_isMaterialChanged = true;
		bool m_isTextureChanged = true;
		bool m_isAlphaClipping = false;

		//-----Layer-----//
	public:
		virtual void AddLayer(uint32 LayerNum) override;
		virtual void DeleteLayer(uint32 LayerNum) override;
		virtual void ChangeLayer(uint32 LayerNum) override;
		virtual void ChangeLayerBitMask(uint32 BitMask) override;

		Ideal::IdealLayer& GetLayer() { return m_Layer; }
		
	private:
		Ideal::IdealLayer m_Layer;
	};
}