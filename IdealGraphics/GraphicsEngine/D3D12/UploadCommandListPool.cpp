#include "UploadCommandListPool.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

void Ideal::CommandListContainer::CloseAndExecute(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Fence> Fence)
{
	CommandList->Close();
	ID3D12CommandList* commandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

Ideal::UploadCommandListPool::UploadCommandListPool()
{

}

Ideal::UploadCommandListPool::~UploadCommandListPool()
{

}

void Ideal::UploadCommandListPool::Init(ComPtr<ID3D12Device> Device, ComPtr<ID3D12Fence> Fence, const uint32& NumCommandList)
{
	m_numContainers = NumCommandList;
	// uploadbufferpool 만들기
	CreateUploadBuffers(Device);

	m_Fence = Fence;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	for (uint32 i = 0; i < NumCommandList; ++i)
	{
		std::shared_ptr<Ideal::CommandListContainer> newContainer = std::make_shared<Ideal::CommandListContainer>();

		std::wstring allocatorName = L"CommandAllocator_" + i;
		std::wstring listName = L"CommandList_" + i;

		Check(
			Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(newContainer->CommandAllocator.GetAddressOf()))
			, L"Failed to create CommandAllocator"
		);
		newContainer->CommandAllocator->SetName(allocatorName.c_str());
		Check(
			Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, newContainer->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(newContainer->CommandList.GetAddressOf()))
			, L"Faield to create CommandList"
		);
		newContainer->CommandList->SetName(listName.c_str());

		Check(
			newContainer->CommandList->Close(),
			L"Failed to close CommandList"
		);

		m_FreedContainer.push_back(newContainer);
	}
}

std::shared_ptr<Ideal::CommandListContainer> Ideal::UploadCommandListPool::AllocateUploadContainer(uint32 Size)
{
	m_countForCheck++;
	if (m_FreedContainer.size() == 0 || m_countForCheck >= UPLOAD_CONTAINER_LOOP_CHECK_COUNT)
	{
		m_countForCheck = 0;
		// TODO 
		// 만약 FreedContainer가 없다면 한 번 루프를 돌면서 찾는다.
		// 만약 없을경우 여기서 Wait를 해야 할 것같다.
		CheckFreedUploadContainer();
	}

	std::shared_ptr<Ideal::CommandListContainer> ret = m_FreedContainer.front();
	m_AllocatedContainer.splice(m_AllocatedContainer.end(), m_FreedContainer, m_FreedContainer.begin());

	// Upload Buffer
	if (Size < 65536)
	{
		ret->UploadBuffer = m_uploadBuffers64kb.top();
		m_uploadBuffers64kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size64KB;
	}
	else if (Size < 65536 * 2)
	{
		ret->UploadBuffer = m_uploadBuffers128kb.top();
		m_uploadBuffers128kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size128KB;
	}
	else if (Size < 65536 * 4)
	{
		ret->UploadBuffer = m_uploadBuffers256kb.top();
		m_uploadBuffers256kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size256KB;
	}
	else if (Size < 65536 * 8)
	{
		ret->UploadBuffer = m_uploadBuffers512kb.top();
		m_uploadBuffers512kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size512KB;
	}
	else if (Size < 4194304)
	{
		ret->UploadBuffer = m_uploadBuffers4096kb.top();
		m_uploadBuffers4096kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size4096KB;
	}
	else if (Size < 41943040)
	{
		ret->UploadBuffer = m_uploadBuffers40960kb.top();
		m_uploadBuffers40960kb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size40960KB;
	}
	else if (Size < 134217728)
	{
		ret->UploadBuffer = m_uploadBuffers128mb.top();
		m_uploadBuffers128mb.pop();
		ret->EUploadBufferSizeType = Ideal::EUploadBufferSize::Size128MB;
	}
	else
	{
		std::wstring message = L"Failed To Create Upload Buffer Container. Size : " + std::to_wstring(Size);
		MyMessageBoxW(message.c_str());
		__debugbreak();
	}
	// Command Reset
	ret->CommandAllocator->Reset();
	ret->CommandList->Reset(ret->CommandAllocator.Get(), nullptr);

	return ret;
}

void Ideal::UploadCommandListPool::CheckFreedUploadContainer()
{
	for (auto it = m_AllocatedContainer.begin(); it != m_AllocatedContainer.end(); )
	{
		std::shared_ptr<Ideal::CommandListContainer> Container = (*it);

		std::string count =std::to_string(m_Fence->GetCompletedValue());
		std::string count2 =std::to_string(Container->FenceValue);
		std::string result = count + " : " + count2 + "\n";
		OutputDebugStringA(result.c_str());
		if (m_Fence->GetCompletedValue() >= Container->FenceValue)
		{
			RevertUploadBuffer(Container);

			// splice(요소를 추가할 위치,  요소를 제거할 원본 리스트, 이동할 요소의 이터레이터)
			// 리스트의 요소를 복사하지 않고 포인터만 재배치하기 때문에 빠르다
			auto nextIt = std::next(it);
			m_FreedContainer.splice(m_FreedContainer.end(), m_AllocatedContainer, it);
			it = nextIt;
		}
		else
		{
			std::string result = "Doing Upload \n";
			OutputDebugStringA(result.c_str());
			++it;
		}
	}

	// 이런데도 없으면 기다려야하나
	if (m_FreedContainer.size() == 0)
	{
		AllWaitForFenceValue();
	}

}

void Ideal::UploadCommandListPool::AllWaitForFenceValue()
{
	for (auto& container : m_AllocatedContainer)
	{
		// wait for fence value
		const uint64 expectedFenceValue = container->FenceValue;
		if (m_Fence->GetCompletedValue() < expectedFenceValue)
		{
			m_Fence->SetEventOnCompletion(expectedFenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);

			std::string result = "Wait in Upload Command List Pool! \n";
			OutputDebugStringA(result.c_str());
		}
	}

	while (m_AllocatedContainer.size())
	{
		auto ptr = m_AllocatedContainer.back();
		RevertUploadBuffer(ptr);
		m_FreedContainer.push_back(ptr);
		m_AllocatedContainer.pop_back();
	}
}

void Ideal::UploadCommandListPool::CreateUploadBuffers(ComPtr<ID3D12Device> Device)
{
	for (uint32 i = 0; i < m_numContainers; ++i)
	{
		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer64 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer64->Create(Device.Get(), 64 * 1024);
		m_uploadBuffers64kb.push(UploadBuffer64);

		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer128 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer128->Create(Device.Get(), 64 * 1024 * 2);
		m_uploadBuffers128kb.push(UploadBuffer128);

		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer256 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer256->Create(Device.Get(), 64 * 1024 * 4);
		m_uploadBuffers256kb.push(UploadBuffer256);

		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer512 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer512->Create(Device.Get(), 64 * 1024 * 8);
		m_uploadBuffers512kb.push(UploadBuffer512);
		
		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer4096 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer4096->Create(Device.Get(), 4194303);
		m_uploadBuffers4096kb.push(UploadBuffer4096);

		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer40960 = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer40960->Create(Device.Get(), 41943040);
		m_uploadBuffers40960kb.push(UploadBuffer40960);

		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer128mb = std::make_shared<Ideal::D3D12UploadBuffer>();
		UploadBuffer128mb->Create(Device.Get(), 134217728);
		m_uploadBuffers128mb.push(UploadBuffer128mb);
	}
}

void Ideal::UploadCommandListPool::RevertUploadBuffer(std::shared_ptr<Ideal::CommandListContainer> Container)
{
	switch (Container->EUploadBufferSizeType)
	{
		case Size64KB:
			m_uploadBuffers64kb.push(Container->UploadBuffer);
			break;
		case Size128KB:
			m_uploadBuffers128kb.push(Container->UploadBuffer);
			break;
		case Size256KB:
			m_uploadBuffers256kb.push(Container->UploadBuffer);
			break;
		case Size512KB:
			m_uploadBuffers512kb.push(Container->UploadBuffer);
			break;
		case Size4096KB:
			m_uploadBuffers4096kb.push(Container->UploadBuffer);
			break;
		case Size40960KB:
			m_uploadBuffers40960kb.push(Container->UploadBuffer);
			break;
		case Size128MB:
			m_uploadBuffers128mb.push(Container->UploadBuffer);
			break;
		default:
			__debugbreak();
			break;
	}

	Container->UploadBuffer = nullptr;
}

