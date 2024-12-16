#include "GraphicsEngine/Resource/UI/IdealCanvas.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12PipelineStateObject.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/VertexInfo.h"
#include "GraphicsEngine/Resource/UI/IdealSprite.h"
#include "GraphicsEngine/Resource/UI/IdealText.h"
#include "GraphicsEngine/D3D12/D3D12DescriptorHeap.h"
#include "GraphicsEngine/D3D12/D3D12ConstantBufferPool.h"
#include "GraphicsEngine/D3D12/D3D12DynamicConstantBufferAllocator.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"

Ideal::IdealCanvas::IdealCanvas()
{

}

Ideal::IdealCanvas::~IdealCanvas()
{

}

void Ideal::IdealCanvas::Init(ComPtr<ID3D12Device> Device)
{
	CreateRootSignature(Device);
	CreatePSO(Device);
}

void Ideal::IdealCanvas::DrawCanvas(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> UIDescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool)
{
	SortSpriteByZ();
	// TODO Set
	CommandList->SetPipelineState(m_rectPSO.Get());
	CommandList->SetGraphicsRootSignature(m_rectRootSignature.Get());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::vector<std::weak_ptr<Ideal::IdealSprite>> transparentSprites;
	for (auto& sprite : m_sprites)
	{
		// Draw Off
		if (sprite.lock()->GetActive() == false)
		{
			continue;
		}

		// Transparent
		float alpha = sprite.lock()->GetAlpha();
		if (alpha < 1.f)
		{
			transparentSprites.push_back(sprite);
			continue;
		}
		sprite.lock()->DrawSprite(Device, CommandList, UIDescriptorHeap, CBPool, Vector2(m_uiCanvasWidth, m_uiCanvasHeight));
	}

	for (auto& text : m_texts)
	{
		text.lock()->UpdateDynamicTextureWithImage(Device);

		auto sprite = text.lock()->GetSprite();
		// Draw Off
		if (sprite->GetActive() == false)
		{
			continue;
		}

		// Transparent
		float alpha = sprite->GetAlpha();
		//if (alpha < 1.f)ㅜ
		{
			// 일단 글자 배경이 투명이라 그냥 투명오브젝트로 넣어준다.
			transparentSprites.push_back(sprite);
			continue;
		}
		//sprite->DrawSprite(Device, CommandList, UIDescriptorHeap, CBPool, Vector2(m_uiCanvasWidth, m_uiCanvasHeight));

	}

	// 투명
	for (auto& sprite : transparentSprites)
	{
		// Draw Off
		if (sprite.lock()->GetActive() == false)
		{
			continue;
		}
		sprite.lock()->DrawSprite(Device, CommandList, UIDescriptorHeap, CBPool, Vector2(m_uiCanvasWidth, m_uiCanvasHeight));
	}
}

void Ideal::IdealCanvas::AddSprite(std::weak_ptr<Ideal::IdealSprite> Sprite)
{
	m_sprites.push_back(Sprite);
}

void Ideal::IdealCanvas::DeleteSprite(std::weak_ptr<Ideal::IdealSprite> Sprite)
{
	if (auto lockedSprite = Sprite.lock())
	{
		auto it = std::find_if(m_sprites.begin(), m_sprites.end(), 
			[&lockedSprite](const std::weak_ptr<Ideal::IdealSprite>& s) 
			{
				return s.lock() == lockedSprite; 
			}
		);
		if (it != m_sprites.end())
		{
			std::swap(*it, m_sprites.back());
			m_sprites.pop_back();
		}
	}
}

void Ideal::IdealCanvas::AddText(std::weak_ptr<Ideal::IdealText> Text)
{
	m_texts.push_back(Text);
}

void Ideal::IdealCanvas::DeleteText(std::weak_ptr<Ideal::IdealText> Text)
{
	if (auto lockedText = Text.lock())
	{
		auto it = std::find_if(m_texts.begin(), m_texts.end(),
			[&lockedText](const std::weak_ptr<Ideal::IdealText>& s)
			{
				return s.lock() == lockedText;
			}
		);
		if (it != m_texts.end())
		{
			std::swap(*it, m_texts.back());
			m_texts.pop_back();
		}
	}
}

void Ideal::IdealCanvas::SetCanvasSize(uint32 Width, uint32 Height)
{
	m_uiCanvasWidth = Width;
	m_uiCanvasHeight = Height;

	for (auto sprite : m_sprites)
	{
		sprite.lock()->ReSize(Width, Height);
	}
	for (auto text : m_texts)
	{
		text.lock()->Resize(Width, Height);
	}
}

void Ideal::IdealCanvas::SortSpriteByZ()
{
	bool IsDirty = false;
	for (auto& sprite : m_sprites)
	{
		if (sprite.lock()->IsDirty())
		{
			sprite.lock()->SetDirty(false);
			IsDirty = true;
			break;
		}
	}
	if (IsDirty)
	{
		IsDirty = false;
		std::sort(m_sprites.begin(), m_sprites.end(), [](const std::weak_ptr<Ideal::IdealSprite>& A, const std::weak_ptr<Ideal::IdealSprite>& B) 
			{ return A.lock()->GetZ() > B.lock()->GetZ(); });
	}
}

void Ideal::IdealCanvas::CreateRootSignature(ComPtr<ID3D12Device> Device)
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::RectRootSignature::Slot::Count];
	ranges[Ideal::RectRootSignature::Slot::SRV_Sprite].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		// t1 : Sprite
	ranges[Ideal::RectRootSignature::Slot::CBV_RectInfo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : RectInfo

	CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::RectRootSignature::Slot::Count];
	rootParameters[Ideal::RectRootSignature::Slot::SRV_Sprite].InitAsDescriptorTable(1, &ranges[Ideal::RectRootSignature::Slot::SRV_Sprite]);
	rootParameters[Ideal::RectRootSignature::Slot::CBV_RectInfo].InitAsDescriptorTable(1, &ranges[Ideal::RectRootSignature::Slot::CBV_RectInfo]);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		// LinearWrapSampler
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC),	// s0
	};

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, staticSamplers, rootSignatureFlags); // (ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
	HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	Check(hr, L"Failed to serialize root signature in ui");
	//HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	//if (error)
	//{
	//	const wchar_t* msg = static_cast<wchar_t*>(error->GetBufferPointer());
	//	Check(hr, msg);
	//}

	hr = (Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_rectRootSignature)));
	Check(hr, L"Failed to create RootSignature");

	m_rectRootSignature->SetName(L"RectRootSignature");
}

void Ideal::IdealCanvas::CreatePSO(ComPtr<ID3D12Device> Device)
{
	// TODO : shader 
	std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	vs->CompileFromFile(L"../Shaders/UI.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	// TODO : shader
	std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	ps->CompileFromFile(L"../Shaders/UI.hlsl", nullptr, nullptr, "PS", "ps_5_0");

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { SimpleVertex::InputElements, SimpleVertex::InputElementCount };
	psoDesc.pRootSignature = m_rectRootSignature.Get();
	psoDesc.VS = vs->GetShaderByteCode();
	psoDesc.PS = ps->GetShaderByteCode();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	{
		// https://codingfarm.tistory.com/609
		// 설정된 블렌드 상태
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;	// one // zero 일경우 검은색으로 바뀌어간다.

		//blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		//blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		psoDesc.BlendState = blendDesc;
	}
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	Check(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_rectPSO.GetAddressOf())));
	return;

	//m_rectPSO = std::make_shared<Ideal::D3D12PipelineStateObject>();
	//m_rectPSO->SetInputLayout(SimpleVertex::InputElements, SimpleVertex::InputElementCount);
	//
	//// TODO : shader 
	//std::shared_ptr<Ideal::D3D12Shader> vs = std::make_shared<Ideal::D3D12Shader>();
	//vs->CompileFromFile(L"../Shaders/UI.hlsl", nullptr, nullptr, "VS", "vs_5_0");
	//m_rectPSO->SetVertexShader(vs);
	//// TODO : shader
	//std::shared_ptr<Ideal::D3D12Shader> ps = std::make_shared<Ideal::D3D12Shader>();
	//ps->CompileFromFile(L"../Shaders/UI.hlsl", nullptr, nullptr, "PS", "ps_5_0");
	//m_rectPSO->SetPixelShader(ps);
	//m_rectPSO->SetRootSignature(m_rectRootSignature.Get());
	//m_rectPSO->SetRasterizerState();
	//m_rectPSO->SetBlendState();
	//
	//DXGI_FORMAT rtvFormat[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	//DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT;
	//m_rectPSO->SetTargetFormat(1, rtvFormat, dsvFormat);
	//m_rectPSO->Create(Device.Get());
}
