#pragma once
#include "Core/Core.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "GraphicsEngine/ConstantBufferInfo.h"

namespace Ideal
{
	class IdealStaticMeshObject;
	class IMeshObject;
	class D3D12Shader;
	class ResourceManager;
	class D3D12StructuredBuffer;
	class D3D12DescriptorHeap2;
	class DeferredDeleteManager;
	class D3D12DynamicConstantBufferAllocator;
}

namespace Ideal
{
	class MeshShaderManager
	{
	public:
		MeshShaderManager();
		~MeshShaderManager() = default;

		// TEST TEST TEST TEST TEST TEST //
		void SetMesh(std::shared_ptr<Ideal::IMeshObject> Mesh, std::shared_ptr<Ideal::ResourceManager> ResourceManager);
		std::shared_ptr<Ideal::IdealStaticMeshObject> m_testMesh;

		std::shared_ptr<Ideal::D3D12StructuredBuffer> m_meshletBuffer;
		std::shared_ptr<Ideal::D3D12StructuredBuffer> m_meshletVertexIndicesBuffer;
		std::shared_ptr<Ideal::D3D12StructuredBuffer> m_meshletTrianglesBuffer;
		uint32 m_meshletCount = 0;

	public:
		void Init(ComPtr<ID3D12Device5> Device);
		void Draw(ComPtr<ID3D12Device5> Device, ComPtr<ID3D12GraphicsCommandList6> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap2> DescriptorHeap, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBAllocator, const CB_Global& CBData);
		
		void SetMS(std::shared_ptr<Ideal::D3D12Shader> Shader);
		void SetPS(std::shared_ptr<Ideal::D3D12Shader> Shader);
	private:
		void CreateRootSignature(ComPtr<ID3D12Device5> Device);
		void CreatePipelineState(ComPtr<ID3D12Device5> Device);

	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		std::shared_ptr<Ideal::D3D12Shader> m_meshShader;
		std::shared_ptr<Ideal::D3D12Shader> m_pixelShader;
	};
}
