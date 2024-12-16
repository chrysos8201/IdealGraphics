#pragma once
#include "IParticleMaterial.h"
#include "Core/Core.h"

struct ID3D12Device;

namespace Ideal
{
	class D3D12Shader;
	class D3D12Texture;
}

/// <summary>
/// 방법 1 
/// UV.z 에 Curve데이터를 넣는 방법
/// BitPacking 을 이용하여 16bit float을 두 개를 넣고 시간에 따라 적절히 사용한다.
/// 이렇게 되면 Root Signature와 PSO를 다른 방식으로 만들.. 필ㅇ가...
/// PSO는 어쨌든 쉐이더 때문에 달라져야 하지만 Vertex의 정보가 통일되고
/// Constant Buffer를 또 추가하지 않아도 된다.
/// 
/// 그렇다면 해야 할 것
/// 기존에 쓰던 UV를 float4로 바꾸고 UV.zw 부터 쓸 수 있게 해야 한다.
/// Vertex Data
/// {
///		Position, Normal, Tangent, UV0, UV1, Color
/// }
/// 
/// 텍스쳐와 이미지에 맞는 UV좌표든 뭐든 있어야 한다.
/// 
/// 방법2
/// constant buffer에 다 때려박는다.
/// </summary>
namespace Ideal
{
	class ParticleMaterial : public IParticleMaterial
	{
	public:
		ParticleMaterial() {};
		virtual ~ParticleMaterial() {};

	public:
		virtual void SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode BlendingMode) override;
		virtual void SetShader(std::shared_ptr<Ideal::IShader> Shader) override;
		virtual void SetShaderByPath(const std::wstring& FilePath) override;
		virtual void SetTexture0(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void SetTexture1(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void SetTexture2(std::shared_ptr<Ideal::ITexture> Texture) override;
		virtual void SetBackFaceCulling(bool Culling) override;
		virtual void SetWriteDepthBuffer(bool Write) override;
		virtual void SetTransparency(bool Transpaerancy) override;
	public:
		Ideal::ParticleMaterialMenu::EBlendingMode GetBlendingMode();
		std::shared_ptr<Ideal::D3D12Shader> GetShader();
		std::weak_ptr<Ideal::D3D12Texture> GetTexture0();
		std::weak_ptr<Ideal::D3D12Texture> GetTexture1();
		std::weak_ptr<Ideal::D3D12Texture> GetTexture2();
		bool GetBackFaceCulling();
		bool GetWriteDepthBuffer();
		bool GetTransparency();

	private:
		Ideal::ParticleMaterialMenu::EBlendingMode m_blendingMode;
		std::shared_ptr<Ideal::D3D12Shader> m_shader;	// 셰이더 시스템
		std::weak_ptr<Ideal::D3D12Texture> m_texture0;	
		std::weak_ptr<Ideal::D3D12Texture> m_texture1;	
		std::weak_ptr<Ideal::D3D12Texture> m_texture2;	
		bool m_isBackFaceCulling = true;
		bool m_isWriteDepthBuffer = false;
		bool m_isTransperancy = false;
	};
}