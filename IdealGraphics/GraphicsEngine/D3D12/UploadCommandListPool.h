#pragma once
#include "Core/Core.h"

struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;
struct ID3D12Fence;
struct ID3D12CommandQueue;

namespace Ideal
{
	class D3D12UploadBuffer;
}

namespace Ideal
{
	enum EUploadBufferSize
	{
		Size64KB,
		Size128KB,
		Size256KB,
		Size512KB,
		Size4096KB,
		Size40960KB,
		Size128MB,
		
	};

	struct CommandListContainer
	{
		std::shared_ptr<Ideal::D3D12UploadBuffer> UploadBuffer;
		ComPtr<ID3D12CommandAllocator> CommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		uint64 FenceValue = 0;
		EUploadBufferSize EUploadBufferSizeType = Size64KB;
		void CloseAndExecute(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Fence> Fence);
		// TODO FenceValue를 항상 복사해서 저장해야한다.

	};

	class UploadCommandListPool
	{
	public:
		UploadCommandListPool();
		~UploadCommandListPool();

	public:
		// commandlist를 여러개 만든다.
		void Init(ComPtr<ID3D12Device> Device, ComPtr<ID3D12Fence> Fence, const uint32& NumCommandList);
		std::shared_ptr<Ideal::CommandListContainer> AllocateUploadContainer(uint32 Size);

		void CheckFreedUploadContainer();

		// 모든 사용되고 있는 컨테이너를 기다린다.
		void AllWaitForFenceValue();

		//
	private:
		void CreateUploadBuffers(ComPtr<ID3D12Device> Device);

		void RevertUploadBuffer(std::shared_ptr<Ideal::CommandListContainer> Container);

	private:
		uint32 m_numContainers = 0;

		uint32 m_countForCheck = 0;


		ComPtr< ID3D12Fence> m_Fence;
		HANDLE m_fenceEvent;
		//std::queue<ComPtr<ID3D12GraphicsCommandList>> m_CommandListsQueue;
		//std::queue<std::shared_ptr<CommandListContainer>> m_CommandListConatinerQueue;
		std::list<std::shared_ptr<Ideal::CommandListContainer>> m_FreedContainer;
		std::list<std::shared_ptr<Ideal::CommandListContainer>> m_AllocatedContainer;

		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers64kb; // 65536
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers128kb;
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers256kb;
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers512kb;
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers4096kb;
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers40960kb;
		std::stack<std::shared_ptr<Ideal::D3D12UploadBuffer>> m_uploadBuffers128mb;
	};
}