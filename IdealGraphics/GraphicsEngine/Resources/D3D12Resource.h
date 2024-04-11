#pragma once
#include "Core/Core.h"
#include "RenderTest/D3D12ThirdParty.h"

class D3D12Resource
{
public:
};


// 업로드용 버퍼. 시스템 메모리에 존재한다.
class D3D12Buffer : public D3D12Resource
{
public:
	D3D12Buffer();
	virtual ~D3D12Buffer();

private:
	ComPtr<ID3D12Resource> m_resource;
};

