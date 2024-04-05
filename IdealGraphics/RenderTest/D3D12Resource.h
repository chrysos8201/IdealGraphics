#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"

#include "Engine/TestGraphics.h"

class VertexBuffer
{
public:
	VertexBuffer(std::shared_ptr<TestGraphics> engine, uint64 size, uint64 stride, const void* initData);
	D3D12_VERTEX_BUFFER_VIEW View() const;
	
	// 버퍼 생성에 성공했는지
	bool IsValid();

private:
	bool m_isValid = false;
	ComPtr<ID3D12Resource> m_buffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_view = {};

	VertexBuffer(const VertexBuffer&) = delete;
	void operator=(const VertexBuffer&) = delete;
};

class ConstantBuffer
{
public:
	ConstantBuffer(std::shared_ptr<TestGraphics> engine, uint64 size);
	bool IsValid();
	D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;	// 버퍼 GPU 주소
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc();	// 상수 버퍼 뷰

	void* GetPtr() const;	// 상수 버퍼에 매핑된 포인터를 반환

	template<typename T>
	T* GetPtr()
	{
		return reinterpret_cast<T*>(GetPtr());
	}

private:
	bool m_isValid = false;
	ComPtr<ID3D12Resource> m_buffer;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_desc;
	void* m_mappedPtr = nullptr;

	ConstantBuffer(const ConstantBuffer&) = delete;
	void operator=(const ConstantBuffer&) = delete;
};

template <typename BufferType>
class ConstantBuffer2
{
public:
	ConstantBuffer2(std::shared_ptr<TestGraphics> engine, uint64 size)
	{

	}

private:
	ComPtr<ID3D12Resource> m_buffer;
	BufferType m_bufferData;
};

//struct ID3D12RootSignature;
class RootSignature
{
public:
	RootSignature(std::shared_ptr<TestGraphics> engine);
	bool IsValid();
	ID3D12RootSignature* Get();

private:
	bool m_isValid = false;
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
};

class PipelineState
{
public:
	PipelineState(std::shared_ptr<TestGraphics> graphics);
	bool IsValid();

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout);
	void SetRootSignature(ID3D12RootSignature* rootSignature);
	void SetVS(std::wstring filePath);
	void SetPS(std::wstring filePath);
	void Create();

	ID3D12PipelineState* Get();

private:
	std::shared_ptr<TestGraphics> m_graphics;

private:
	bool m_isValid = false;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;
};