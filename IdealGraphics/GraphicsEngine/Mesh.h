#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12Resource.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"
#include "GraphicsEngine/VertexInfo.h"

class IdealRenderer;
class AssimpLoader;

namespace Ideal
{
	class Mesh
	{
		friend class AssimpLoader;
		friend class AssimpConverter;

	public:
		Mesh();
		virtual ~Mesh();

	public:
		void Create(std::shared_ptr<IdealRenderer> Renderer);

		// Temp
		void Tick();

		void Render(ID3D12GraphicsCommandList* CommandList);

	private:
		void InitRootSignature();
		void InitPipelineState();

	private:
		std::shared_ptr<IdealRenderer> m_renderer;

		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12RootSignature> m_rootSignature;

		D3D12VertexBuffer m_vertexBuffer;
		D3D12IndexBuffer m_indexBuffer;
		D3D12ConstantBuffer m_constantBuffer;

		Transform m_transform;
		Transform* m_cbDataBegin;

	private:
		std::string m_name;
		int32 m_boneIndex;	//���� ��� �ε��� ��
		std::string m_materialName;	// �� mesh�� �׸��� �ʿ��� material�� �̸��� ������ �ְڴ�.

		std::vector<BasicVertex> m_vertices;	// ������ ���� �ʿ䰡 ����
		std::vector<uint32> m_indices;	// ��������

		std::wstring m_diffuseMap;
	};
}