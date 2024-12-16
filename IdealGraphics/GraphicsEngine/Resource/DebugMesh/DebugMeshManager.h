#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/ConstantBufferInfo.h"

struct ID3D12Device;
struct ID3D12RootSignature;
struct ID3D12GraphicsCommandList;
struct ID3D12PipelineState;

namespace Ideal
{
	class D3D12DescriptorHeap;
	class D3D12DynamicConstantBufferAllocator;
	class IdealStaticMeshObject;
	class D3D12Shader;
	class D3D12VertexBuffer;
	class D3D12UploadBuffer;
}

namespace Ideal
{
	class DebugMeshManager
	{
		struct DebugLine
		{
			Vector3 StartPos;
			float pad0;
			Vector3 EndPos;
			float pad1;
			Color Color;
		};
	public:
		void Init(ComPtr<ID3D12Device> Device);
		void SetDebugLineVB(std::shared_ptr<Ideal::D3D12VertexBuffer> VB);

	private:
		void CreateRootSignatureDebugMesh(ComPtr<ID3D12Device> Device);
		void CreatePipelineStateDebugMesh(ComPtr<ID3D12Device> Device);

		void CreateRootSignatureDebugLine(ComPtr<ID3D12Device> Device);
		void CreatePipelineStateDebugLine(ComPtr<ID3D12Device> Device);

	public:
		void AddDebugMesh(std::shared_ptr<Ideal::IdealStaticMeshObject> DebugMesh);
		void DeleteDebugMesh(std::shared_ptr<Ideal::IdealStaticMeshObject> DebugMesh);

		void SetVS(std::shared_ptr<Ideal::D3D12Shader> Shader);
		void SetPS(std::shared_ptr<Ideal::D3D12Shader> Shader);

		void SetVSLine(std::shared_ptr<Ideal::D3D12Shader> Shader);
		void SetPSLine(std::shared_ptr<Ideal::D3D12Shader> Shader);

		void DrawDebugMeshes(ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CommandList, std::shared_ptr<Ideal::D3D12DescriptorHeap> DescriptorHeap, std::shared_ptr<Ideal::D3D12DynamicConstantBufferAllocator> CBPool, CB_Global* CB_GlobalData);

		void AddDebugLine(Vector3 start, Vector3 end, Color Color = Color(1,0,0));
		//void DebugLineInstanceBake(ComPtr<ID3D12Device> Device, std::shared_ptr<Ideal::ResourceManager> ResourceManager);

	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;

		ComPtr<ID3D12RootSignature> m_rootSignatureDebugLine;
		ComPtr<ID3D12PipelineState> m_pipelineStateDebugLine;

		std::shared_ptr<Ideal::D3D12Shader> m_vs;
		std::shared_ptr<Ideal::D3D12Shader> m_ps;
		
		std::shared_ptr<Ideal::D3D12Shader> m_vsLine;
		std::shared_ptr<Ideal::D3D12Shader> m_psLine;
		std::shared_ptr<Ideal::D3D12VertexBuffer> m_lineVB;

	private:
		std::vector<std::weak_ptr<Ideal::IdealStaticMeshObject>> m_meshes;
		std::vector<CB_DebugLine> m_lines;
		std::shared_ptr<Ideal::D3D12UploadBuffer> m_instanceBuffer;
	};
}
