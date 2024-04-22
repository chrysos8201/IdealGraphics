#include "GraphicsEngine/D3D12/D3D12ResourceManager.h"

#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/VertexInfo.h"

using namespace Ideal;

D3D12ResourceManager::D3D12ResourceManager()
{

}

D3D12ResourceManager::~D3D12ResourceManager()
{
	WaitForFenceValue();
}

void D3D12ResourceManager::Init(ComPtr<ID3D12Device> Device)
{
	m_device = Device;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	Check(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf())));

	//-----------Create Command Allocator---------//
	Check(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));

	//-----------Create Command List---------//
	Check(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));
	m_commandList->Close();

	//-----------Create Fence---------//
	Check(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
	m_fenceValue = 0;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_fence->SetName(L"ResourceManager Fence");
}

void D3D12ResourceManager::Fence()
{
	m_fenceValue++;
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
}

void D3D12ResourceManager::WaitForFenceValue()
{
	const uint64 expectedFenceValue = m_fenceValue;

	if (m_fence->GetCompletedValue() < expectedFenceValue)
	{
		m_fence->SetEventOnCompletion(expectedFenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D12ResourceManager::CreateVertexBufferBox(std::shared_ptr<Ideal::D3D12VertexBuffer>& VertexBuffer)
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;	// 업로드 용으로 버퍼 하나를 만드는 것 같다.
	const VertexTest2 cubeVertices[] = {
		{ { -0.5f, 0.5f, -0.5f},	Vector2(0.0f, 1.0f) },    // Back Top Left
		{ {  0.5f, 0.5f, -0.5f},	Vector2(0.0f, 0.0f) },    // Back Top Right
		{ {  0.5f, 0.5f,  0.5f},	Vector2(1.0f, 0.0f) },    // Front Top Right
		{ { -0.5f, 0.5f,  0.5f},	Vector2(1.0f, 1.0f)	},    // Front Top Left

		{ { -0.5f, -0.5f,-0.5f},	Vector2(0.0f, 1.0f)	},    // Back Bottom Left
		{ {  0.5f, -0.5f,-0.5f},	Vector2(0.0f, 0.0f) },    // Back Bottom Right
		{ {  0.5f, -0.5f, 0.5f},	Vector2(1.0f, 0.0f) },    // Front Bottom Right
		{ { -0.5f, -0.5f, 0.5f},	Vector2(1.0f, 1.0f) },    // Front Bottom Left
	};
	const uint32 vertexBufferSize = sizeof(cubeVertices);

	// UploadBuffer를 만든다.
	Ideal::D3D12UploadBuffer uploadBuffer;
	uploadBuffer.Create(m_device.Get(), vertexBufferSize);

	void* mappedUploadHeap = uploadBuffer.Map();
	memcpy(mappedUploadHeap, cubeVertices, sizeof(cubeVertices));
	uploadBuffer.UnMap();

	uint32 vertexTypeSize = sizeof(VertexTest2);
	uint32 vertexCount = _countof(cubeVertices);
	VertexBuffer->Create(m_device.Get(), m_commandList.Get(), vertexTypeSize, vertexCount, uploadBuffer);

	// close
	m_commandList->Close();

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	VertexBuffer->SetName(L"TestBoxVB");

	Fence();
	WaitForFenceValue();
}

void D3D12ResourceManager::CreateIndexBufferBox(std::shared_ptr<Ideal::D3D12IndexBuffer> IndexBuffer)
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	uint32 indices[] = {
			0, 1, 3,
			1, 2, 3,

			3, 2, 7,
			6, 7, 2,

			2, 1, 6,
			5, 6, 1,

			1, 0, 5,
			4, 5, 0,

			0, 3, 4,
			7, 4, 3,

			7, 6, 4,
			5, 4, 6, };
	const uint32 indexBufferSize = sizeof(indices);

	Ideal::D3D12UploadBuffer uploadBuffer;
	uploadBuffer.Create(m_device.Get(), indexBufferSize);


	// 업로드버퍼에 먼저 복사
	void* mappedUploadBuffer = uploadBuffer.Map();
	memcpy(mappedUploadBuffer, indices, indexBufferSize);
	uploadBuffer.UnMap();

	uint32 indexTypeSize = sizeof(uint32);
	uint32 indexCount = _countof(indices);

	IndexBuffer->Create(m_device.Get(), m_commandList.Get(), indexTypeSize, indexCount, uploadBuffer);

	// close
	m_commandList->Close();

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	IndexBuffer->SetName(L"TestBoxIB");

	Fence();
	WaitForFenceValue();
}

void D3D12ResourceManager::CreateVertexBufferTest(UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData)
{
	ID3D12Device* m_pD3DDevice = m_device.Get();
	ID3D12CommandAllocator* m_pCommandAllocator = m_commandAllocator.Get();
	ID3D12GraphicsCommandList* m_pCommandList = m_commandList.Get();
	ID3D12CommandQueue* m_pCommandQueue = m_commandQueue.Get();

	HRESULT hr = S_OK;

	D3D12_VERTEX_BUFFER_VIEW	VertexBufferView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	UINT		VertexBufferSize = SizePerVertex * dwVertexNum;

	// create vertexbuffer for rendering
	hr = m_pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&pVertexBuffer));

	if (FAILED(hr))
	{
		__debugbreak();
		goto lb_return;
	}
	if (pInitData)
	{
		if (FAILED(m_pCommandAllocator->Reset()))
			__debugbreak();

		if (FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
			__debugbreak();

		hr = m_pD3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer));

		if (FAILED(hr))
		{
			__debugbreak();
			goto lb_return;
		}

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin = nullptr;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.

		hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (FAILED(hr))
		{
			__debugbreak();
			goto lb_return;
		}
		memcpy(pVertexDataBegin, pInitData, VertexBufferSize);
		pUploadBuffer->Unmap(0, nullptr);

		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		m_pCommandList->CopyBufferRegion(pVertexBuffer, 0, pUploadBuffer, 0, VertexBufferSize);
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		m_pCommandList->Close();

		ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
		m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		Fence();
		WaitForFenceValue();
	}


	// Initialize the vertex buffer view.
	VertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = SizePerVertex;
	VertexBufferView.SizeInBytes = VertexBufferSize;

	*pOutVertexBufferView = VertexBufferView;
	*ppOutBuffer = pVertexBuffer;

lb_return:
	if (pUploadBuffer)
	{
		pUploadBuffer->Release();
		pUploadBuffer = nullptr;
	}
}

