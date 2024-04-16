#include "GraphicsEngine/Mesh.h"

#include "GraphicsEngine/IdealRenderer.h"

Ideal::Mesh::Mesh()
{

}

Ideal::Mesh::~Mesh()
{
	//m_renderer = nullptr;
}

void Ideal::Mesh::Create(std::shared_ptr<IdealRenderer> Renderer)
{
	//--------------Init---------------//
	m_renderer = Renderer;
	InitRootSignature();
	InitPipelineState();

	//--------------VB---------------//
	{
		const uint32 vertexBufferSize = m_vertices.size() * sizeof(Vertex);

		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(Renderer->GetDevice().Get(), vertexBufferSize);
		{
			void* mappedUploadHeap = uploadBuffer.Map();
			//std::copy(m_vertices.begin(), m_vertices.end(), &mappedUploadHeap);
			memcpy(mappedUploadHeap, m_vertices.data(), vertexBufferSize);
			uploadBuffer.UnMap();
		}
		const uint32 elementSize = sizeof(Vertex);
		const uint32 elementCount = m_vertices.size();

		m_vertexBuffer.Create(
			m_renderer->GetDevice().Get(),
			m_renderer->GetCommandList().Get(),
			elementSize,
			elementCount,
			uploadBuffer
		);

		// Wait
		m_renderer->ExecuteCommandList();
	}

	//--------------IB---------------//
	{
		const uint32 indexBufferSize = m_indices.size() * sizeof(uint32);

		Ideal::D3D12UploadBuffer uploadBuffer;
		uploadBuffer.Create(Renderer->GetDevice().Get(), indexBufferSize);
		{
			void* mappedUploadHeap = uploadBuffer.Map();
			//std::copy(m_indices.begin(), m_indices.end(), mappedUploadHeap);
			memcpy(mappedUploadHeap, m_indices.data(), indexBufferSize);
			uploadBuffer.UnMap();
		}
		const uint32 elementSize = sizeof(uint32);
		const uint32 elementCount = m_indices.size();

		m_indexBuffer.Create(
			m_renderer->GetDevice().Get(),
			m_renderer->GetCommandList().Get(),
			elementSize,
			elementCount,
			uploadBuffer
		);

		// Wait
		m_renderer->ExecuteCommandList();
	}

	//--------------CB---------------//
	{
		const uint32 bufferSize = sizeof(Transform);

		m_constantBuffer.Create(m_renderer->GetDevice().Get(), bufferSize, IdealRenderer::FRAME_BUFFER_COUNT);
	}

	//--------------Test---------------//
	m_transform.World = Matrix::Identity;
	m_transform.World = Matrix::CreateRotationY(DirectX::XMConvertToRadians(37.5f)) * Matrix::CreateTranslation(Vector3(0.f, 0.f, -800.f));

	m_transform.WorldInvTranspose = m_transform.World.Invert().Transpose();//
	m_transform.View = m_renderer->GetView();// .Transpose();
	m_transform.Proj = m_renderer->GetProj();// .Transpose();

	m_vertices.clear();
	m_indices.clear();
}

void Ideal::Mesh::Tick()
{
	static float rot = 0.f;
	rot += 0.2f;
	m_transform.World = Matrix::Identity;
	m_transform.World = Matrix::CreateRotationY(DirectX::XMConvertToRadians(rot)) * Matrix::CreateTranslation(Vector3(0.f, 0.f, -800.f));

	Transform* t = (Transform*)m_constantBuffer.GetMappedMemory(m_renderer->GetFrameIndex());
	*t = m_transform;


}

void Ideal::Mesh::Render(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->SetPipelineState(m_pipelineState.Get());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = m_vertexBuffer.GetView();
	CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = m_indexBuffer.GetView();
	CommandList->IASetIndexBuffer(&indexBufferView);

	CommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	uint32 currentIndex = m_renderer->GetFrameIndex();
	CommandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer.GetGPUVirtualAddress(currentIndex));


	CommandList->DrawIndexedInstanced(m_indexBuffer.GetElementCount(), 1, 0, 0, 0);
}

void Ideal::Mesh::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];

	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	Check(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	Check(m_renderer->GetDevice()->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.GetAddressOf())
	));
}

void Ideal::Mesh::InitPipelineState()
{
	ComPtr<ID3DBlob> errorBlob;
	uint32 compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	// Set VS
	ComPtr<ID3DBlob> vertexShader;
	Check(D3DCompileFromFile(
		L"Shaders/BoxVS.hlsl",
		nullptr,
		nullptr,
		"VS",
		"vs_5_0",
		compileFlags, 0, &vertexShader, nullptr));

	// Set PS
	ComPtr<ID3DBlob> pixelShader;
	Check(D3DCompileFromFile(
		L"Shaders/BoxPS.hlsl",
		nullptr,
		nullptr,
		"PS",
		"ps_5_0",
		compileFlags, 0, &pixelShader, nullptr));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	D3D12_INPUT_ELEMENT_DESC PipelineStateInputLayouts[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Set Input Layout
	psoDesc.InputLayout = { PipelineStateInputLayouts, _countof(PipelineStateInputLayouts)};

	psoDesc.pRootSignature = m_rootSignature.Get();

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	// Set RasterizerState
	//psoDesc.RasterizerState.FillMode = ;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//psoDesc.DepthStencilState.DepthEnable = TRUE;
	//psoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	Check(m_renderer->GetDevice()->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(m_pipelineState.GetAddressOf())
	));
}
