#include "GraphicsEngine/D3D12/MeshShader/MeshShaderManager.h"
#include "GraphicsEngine/D3D12/D3D12Shader.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

#include "GraphicsEngine/Resource/IdealStaticMeshObject.h"
#include "GraphicsEngine/Resource/IdealMesh.h"
#include "Misc/MeshOptimizer/meshoptimizer.h"

Ideal::MeshShaderManager::MeshShaderManager()
{

}

void Ideal::MeshShaderManager::SetMesh(std::shared_ptr<Ideal::IMeshObject> Mesh)
{
	/////////////// TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST////////////
	m_testMesh = std::static_pointer_cast<Ideal::IdealStaticMeshObject>(Mesh);
	auto getMesh = m_testMesh->GetMeshByIndex(0);
	auto castedMesh = std::static_pointer_cast<Ideal::IdealMesh<BasicVertex>>(getMesh.lock());

	auto vertices = castedMesh->GetVerticesRef();
	auto indices = castedMesh->GetIndicesRef();

	// Meshlet Generation
	uint32 maxVertices = 64;
	uint32 maxTriangles = 128;

	const std::size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);

	std::vector<meshopt_Meshlet> meshoptMeshlets;
	std::vector<uint32> meshletVertexIndices;
	std::vector<uint8> meshletTriangles;
	meshoptMeshlets.resize(maxMeshlets);
	meshletVertexIndices.resize(maxMeshlets * maxVertices);
	meshletTriangles.resize(maxMeshlets * maxVertices * 3);
	
	const auto count = meshopt_buildMeshlets(
		meshoptMeshlets.data(),
		meshletVertexIndices.data(),
		meshletTriangles.data(),
		indices.data(),
		indices.size(),
		(const float*)(vertices.data()) + offsetof(BasicVertex, Position)/sizeof(float),
		vertices.size(),
		sizeof(BasicVertex),
		maxVertices,
		maxTriangles,
		0.f	// occlusion culling을 위해 필요하다고 함.
	);

	int a = 3;
}

void Ideal::MeshShaderManager::Init(ComPtr<ID3D12Device5> Device)
{
	CreateRootSignature(Device);
	CreatePipelineState(Device);
}

void Ideal::MeshShaderManager::Draw(ComPtr<ID3D12GraphicsCommandList6> CommandList)
{
	CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
	CommandList->SetPipelineState(m_pipelineState.Get());
	CommandList->DispatchMesh(1, 1, 1);
}

void Ideal::MeshShaderManager::SetMS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_meshShader = Shader;
}

void Ideal::MeshShaderManager::SetPS(std::shared_ptr<Ideal::D3D12Shader> Shader)
{
	m_pixelShader = Shader;
}

void Ideal::MeshShaderManager::CreateRootSignature(ComPtr<ID3D12Device5> Device)
{
	// Create Root Signature
	// Root Signature

	//CD3DX12_DESCRIPTOR_RANGE1 ranges[Ideal::MeshShaderRootSignature::Slot::Count];
	//CD3DX12_ROOT_PARAMETER1 rootParameters[Ideal::MeshShaderRootSignature::Slot::Count];
	//
	//ranges[Ideal::MeshShaderRootSignature::Slot::CBV_Global].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//rootParameters[Ideal::MeshShaderRootSignature::Slot::CBV_Global].InitAsDescriptorTable(1, &ranges[Ideal::MeshShaderRootSignature::Slot::CBV_Global], D3D12_SHADER_VISIBILITY_ALL);
	//
	//ComPtr<ID3DBlob> blob;
	//ComPtr<ID3DBlob> error;
	//D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	//
	//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
	//
	//HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	//Check(hr, L"Failed to serialize root signature Mesh Shader");
	//
	//hr = Device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
	//Check(hr, L"Failed To Create Mesh Shader RootSignature!");

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
	Check(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error));
	Check(Device->CreateRootSignature(
		0,                         // nodeMask
		blob->GetBufferPointer(),  // pBloblWithRootSignature
		blob->GetBufferSize(),     // blobLengthInBytes
		IID_PPV_ARGS(m_rootSignature.GetAddressOf()))); // riid, ppvRootSignature
}

void Ideal::MeshShaderManager::CreatePipelineState(ComPtr<ID3D12Device5> Device)
{
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.MS = m_meshShader->GetShaderByteCode();
	psoDesc.PS = m_pixelShader->GetShaderByteCode();
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.SampleDesc = DefaultSampleDesc();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
	//psoDesc.pRootSignature = m_rootSignature.Get();
	//psoDesc.MS = m_meshShader->GetShaderByteCode();
	//psoDesc.PS = m_pixelShader->GetShaderByteCode();
	//psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	//psoDesc.BlendState.IndependentBlendEnable = FALSE;
	//psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
	//psoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	//psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_COLOR;
	//psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	//psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	//psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	//psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	//psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//psoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	//psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	//psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	//psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	//psoDesc.RasterizerState.DepthClipEnable = FALSE;
	//psoDesc.RasterizerState.MultisampleEnable = FALSE;
	//psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	//psoDesc.RasterizerState.ForcedSampleCount = 0;
	//psoDesc.DepthStencilState.DepthEnable = FALSE;
	//psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	//psoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	//psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	//psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	//psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;
	//psoDesc.NumRenderTargets = 1;
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//psoDesc.SampleDesc.Count = 1;
	// 아래의 설정을 반드시 해주자..
	//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	
	Check(Device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()))
		,L"Failed To Create Mesh Shader Pipeline State!");
}
