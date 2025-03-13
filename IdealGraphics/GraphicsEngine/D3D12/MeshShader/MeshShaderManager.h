#pragma once
#include "Core/Core.h"
#include <d3d12.h>
#include <d3dx12.h>

namespace Ideal
{
	class IdealStaticMeshObject;
	class IMeshObject;
	class D3D12Shader;
}

namespace Ideal
{
	class MeshShaderManager
	{
	public:
		MeshShaderManager();
		~MeshShaderManager() = default;

		// TEST TEST TEST TEST TEST TEST //
		void SetMesh(std::shared_ptr<Ideal::IMeshObject> Mesh);
		std::shared_ptr<Ideal::IdealStaticMeshObject> m_testMesh;

	public:
		void Init(ComPtr<ID3D12Device5> Device);
		void Draw(ComPtr<ID3D12GraphicsCommandList6> CommandList);
		
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
