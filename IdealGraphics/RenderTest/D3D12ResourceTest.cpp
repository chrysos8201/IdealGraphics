#include "RenderTest/D3D12ResourceTest.h"
//#include "ThirdParty/DirectXTex/DirectXTex.h"

VertexBuffer::VertexBuffer(std::shared_ptr<TestGraphics> engine, uint64 size, uint64 stride, const void* initData)
{
	
	CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);

	HRESULT hr = engine->GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_buffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		assert(false);
		return;
	}

	// 정점 버퍼 뷰 설정
	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.SizeInBytes = static_cast<uint32>(size);
	m_view.StrideInBytes = static_cast<uint32>(stride);

	if (initData != nullptr)
	{
		void* ptr = nullptr;
		hr = m_buffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			assert(false);
			return;
		}

		memcpy(ptr, initData, size);

		m_buffer->Unmap(0, nullptr);
	}

	m_isValid = true;
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::View() const
{
	return m_view;
}

bool VertexBuffer::IsValid()
{
	return m_isValid;
}

////////////////////////////////////////////

IndexBuffer::IndexBuffer(std::shared_ptr<TestGraphics> engine, uint64 size, const void* initData)
{
	// desc heap을 만들어야한다.
	// upload heap
	CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);

	HRESULT hr = engine->GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		//D3D12_RESOURCE_STATE_INDEX_BUFFER,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_buffer.GetAddressOf())
	);
	if (FAILED(hr))
	{
		assert(false);
		return;
	}
	// index buffer view 설정?
	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.Format = DXGI_FORMAT_R32_UINT;
	m_view.SizeInBytes = static_cast<uint32>(size);

	if (initData != nullptr)
	{
		void* ptr = nullptr;
		hr = m_buffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			assert(false);
			return;
		}

		memcpy(ptr, initData, size);

		m_buffer->Unmap(0, nullptr);
	}

	m_isValid = true;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::View() const
{
	return m_view;
}

bool IndexBuffer::IsValid()
{
	return m_isValid;
}

////////////////////////////////////////////////

ConstantBuffer::ConstantBuffer(std::shared_ptr<TestGraphics> engine, uint64 size) 
{
	// 상수 버퍼의 크기는 반드시 최소 하드웨어 할당 크기 (흔히 256바이트)의 배수여야 한다.
	uint64 align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	uint64 sizeAligned = (size + (align - 1)) & ~(align - 1);	// align으로 반올림


	CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeAligned);

	auto hr = engine->GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_buffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		assert(false);
		return;
	}

	hr = m_buffer->Map(0, nullptr, &m_mappedPtr);
	if (FAILED(hr))
	{
		assert(false);
		return;
	}
	
	// UnMap이 없는데?
	//m_buffer->Unmap(0, nullptr);

	m_desc = {};
	m_desc.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_desc.SizeInBytes = uint32(sizeAligned);
	
	m_isValid = true;
}

bool ConstantBuffer::IsValid()
{
	return m_isValid;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetAddress() const
{
	return m_desc.BufferLocation;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBuffer::ViewDesc()
{
	return m_desc;
}

void* ConstantBuffer::GetPtr() const
{
	return m_mappedPtr;
}

//////////////////////////////////////////

RootSignature::RootSignature(std::shared_ptr<TestGraphics> engine)
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_PARAMETER rootParam[1] = {};
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // b0에 상수 버퍼 설정

	CD3DX12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

	// 루트 시그니쳐 설정. 
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam);	// 루트 파라미터 개수
	desc.NumStaticSamplers = 1;	// 샘플러의 개수
	desc.pParameters = rootParam;	// 루트 파라미터 포인터
	desc.pStaticSamplers = &sampler;	// 샘플러 포인터
	desc.Flags = flag;	// 플래그 설정

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> errorBlob;

	// 직렬화 serialization
	HRESULT hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		blob.GetAddressOf(),
		errorBlob.GetAddressOf()
	);

	if (FAILED(hr))
	{
		assert(false);
		return;
	}
	
	// 루트 시그니쳐 생성
	hr = engine->GetDevice()->CreateRootSignature(
		0,	// gpu가 여러개 있을 경우
		blob->GetBufferPointer(),	// 직렬화된 데이터의 포인터
		blob->GetBufferSize(),		// 직렬화된 데이터의 크기
		IID_PPV_ARGS(m_rootSignature.GetAddressOf())	// 루트 시그니처를 저장할 포인터
	);

	if (FAILED(hr))
	{
		assert(false);
		return;
	}

	m_isValid = true;
}

bool RootSignature::IsValid()
{
	return m_isValid;
}

ID3D12RootSignature* RootSignature::Get()
{
	return m_rootSignature.Get();
}

////////////////////////////////////////////////////////////////////

PipelineState::PipelineState(std::shared_ptr<TestGraphics> graphics)
	: m_graphics(graphics)
{
	// 파이프라인 스테이트
	ZeroMemory(&desc, sizeof(desc));
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
}

bool PipelineState::IsValid()
{
	return m_isValid;
}

void PipelineState::SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout)
{
	desc.InputLayout = layout;
}

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	desc.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath)
{
	HRESULT hr = D3DReadFileToBlob(filePath.c_str(), m_vsBlob.GetAddressOf());
	if (FAILED(hr))
	{
		assert(false);
		return;
	}

	desc.VS = CD3DX12_SHADER_BYTECODE(m_vsBlob.Get());
}

void PipelineState::SetPS(std::wstring filePath)
{
	HRESULT hr = D3DReadFileToBlob(filePath.c_str(), m_psBlob.GetAddressOf());
	if (FAILED(hr))
	{
		assert(false);
		return;
	}

	desc.PS = CD3DX12_SHADER_BYTECODE(m_psBlob.Get());
}

void PipelineState::Create()
{
	HRESULT hr = m_graphics->GetDevice()->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(m_pipelineState.GetAddressOf())
	);
	if (FAILED(hr))
	{
		assert(false);
		return;
	}
	m_isValid = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return m_pipelineState.Get();
}

////////////////////////////////////////////////////////////////////

const uint32 HANDLE_MAX = 512;

DescriptorHeap::DescriptorHeap(std::shared_ptr<TestGraphics> graphics)
{
// 	m_graphics = graphics;
// 
// 	m_handles.clear();
// 	m_handles.reserve(HANDLE_MAX);
// 
// 	D3D12_DESCRIPTOR_HEAP_DESC desc{};
// 	desc.NodeMask = 1;
// 	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
// 	desc.NumDescriptors = HANDLE_MAX;
// 	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
// 
// 	HRESULT hr = graphics->GetDevice()->CreateDescriptorHeap(
// 		&desc,
// 		IID_PPV_ARGS(m_heap.ReleaseAndGetAddressOf())
// 	);
// 
// 	if (FAILED(hr))
// 	{
// 		m_isValid = false;
// 		return;
// 	}
// 
// 	m_incrementSize = graphics->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
// 	m_isValid = true;
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
	return m_heap.Get();
}

DescriptorHandle* DescriptorHeap::Register(Texture2D* texture)
{
// 	uint64 count = m_handles.size();
// 	if (HANDLE_MAX <= count)
// 	{
// 		return nullptr;
// 	}
// 
// 	DescriptorHandle* handle = new DescriptorHandle();
// 
// 	auto handleCPU = m_heap->GetCPUDescriptorHandleForHeapStart();
// 	handleCPU.ptr += m_incrementSize * count;
// 
// 	auto handleGPU = m_heap->GetGPUDescriptorHandleForHeapStart();
// 	handleGPU.ptr += m_incrementSize * count;
// 
// 	handle->HandleCPU = handleCPU;
// 	handle->HandleGPU = handleGPU;
// 
// 	ID3D12Resource* resource = texture->Resource();
// 	auto desc = texture->ViewDesc();
// 	
// 	m_graphics->GetDevice()->CreateShaderResourceView(resource, &desc, handle->HandleCPU);
// 
// 	m_handles.push_back(handle);
//	return handle;

	return nullptr;
}

//////////////////////////////////////////////

Texture2D* Texture2D::Get(std::string path)
{
	return nullptr;
}

Texture2D* Texture2D::GetWhite()
{
	return nullptr;
}

bool Texture2D::IsValid()
{
	return false;
}

ID3D12Resource* Texture2D::Resource()
{
	return nullptr;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture2D::ViewDesc()
{
	return D3D12_SHADER_RESOURCE_VIEW_DESC();
}

Texture2D::Texture2D(std::string path)
{
/*	m_isValid = Load(path);*/
}

Texture2D::Texture2D(ID3D12Resource* buffer)
{
// 	m_resource = buffer;
// 	m_isValid = m_resource != nullptr;
}

bool Texture2D::Load(std::string& path)
{
	/*using namespace DirectX;
	TexMetadata meta = {};
	ScratchImage scratch = {};
	
	HRESULT hr;

	std::wstring wpath;
	wpath.assign(path.begin(), path.end());
	LoadFromWICFile(wpath.c_str(), WIC_FLAGS_NONE, &meta, scratch);

	if (FAILED(hr))
	{
		return false;
	}

	auto img = scratch.GetImage(0, 0, 0);
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
		meta.format,
		meta.width,
		meta.height,
		static_cast<uint16>(meta.arraySize),
		static_cast<uint16>(meta.mipLevels)
	);*/

	//hr = 

	return false;
}

ID3D12Resource* Texture2D::GetDefaultResource(uint64 width, uint64 height)
{
	return nullptr;
}
