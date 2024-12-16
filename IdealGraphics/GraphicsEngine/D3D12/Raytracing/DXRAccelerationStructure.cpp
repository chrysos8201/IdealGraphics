#include "DXRAccelerationStructure.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include "GraphicsEngine/D3D12/DeferredDeleteManager.h"
#include "GraphicsEngine/Resource/IdealMaterial.h"

inline void AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Check(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
}

Ideal::DXRBottomLevelAccelerationStructure::DXRBottomLevelAccelerationStructure(const std::wstring& Name)
{
	m_name = Name;
}

Ideal::DXRBottomLevelAccelerationStructure::~DXRBottomLevelAccelerationStructure()
{
	
}

void Ideal::DXRBottomLevelAccelerationStructure::Create(ComPtr<ID3D12Device5> Device, std::vector<BLASGeometry>& Geometries, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags, bool AllowUpdate)
{
	m_allowUpdate = AllowUpdate;

	m_buildFlags = BuildFlags;
	if (AllowUpdate)
		m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	// build geometry desc
	BuildGeometries(Geometries);

	// Prebuild
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
		bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bottomLevelInputs.Flags = m_buildFlags;
		bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelInputs.NumDescs = static_cast<uint32>(m_geometryDescs.size());
		bottomLevelInputs.pGeometryDescs = m_geometryDescs.data();

		Device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &m_preBuildInfo);
		if (m_preBuildInfo.ResultDataMaxSizeInBytes <= 0) __debugbreak();
	}

	m_accelerationStructure = std::make_shared<Ideal::D3D12UAVBuffer>();
	m_accelerationStructure->Create(
		Device.Get(),
		m_preBuildInfo.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		m_name.c_str()
	);
	m_isDirty = true;
	m_isBuilt = false;
}

void Ideal::DXRBottomLevelAccelerationStructure::FreeMyHandle()
{
	// Fixed Descriptor 에서 가져다 쓴 리소스들을 반환한다.
	for (auto geometry : m_geometries)
	{
		geometry.SRV_IndexBuffer.Free();
		geometry.SRV_VertexBuffer.Free();
		//geometry.SRV_Diffuse.Free();
		//geometry.SRV_Normal.Free();
		//geometry.SRV_Metallic.Free();
		//geometry.SRV_Roughness.Free();
		//geometry.SRV_Mask.Free();
		//geometry.CBV_MaterialInfo.Free();
		//geometry.Material->Free();
	}

	m_geometries.clear();
}

void Ideal::DXRBottomLevelAccelerationStructure::Build(ComPtr<ID3D12GraphicsCommandList4> CommandList, ComPtr<ID3D12Resource> ScratchBuffer)
{
	// scratch 버퍼
	// 
	uint64 scratchBufferSizeInBytes = ScratchBuffer->GetDesc().Width;
	if (m_preBuildInfo.ScratchDataSizeInBytes > scratchBufferSizeInBytes)
	{
		std::wstring msg = L"Failed to Build BLAS : ";
		msg += m_name;
		MyMessageBoxW(msg.c_str());
		__debugbreak();
	}

	m_currentID = (m_currentID + 1) % MAX_PENDING_COUNT;
	m_cacheGeometryDescs[m_currentID].clear();
	m_cacheGeometryDescs[m_currentID].resize(m_geometryDescs.size());
	copy(m_geometryDescs.begin(), m_geometryDescs.end(), m_cacheGeometryDescs[m_currentID].begin());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& blasInputs = blasBuildDesc.Inputs;

	blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	blasInputs.Flags = m_buildFlags;

	// 이미 빌드되었을 경우만 아래의 플래그 조건 성립
	if (m_isBuilt && m_allowUpdate)
	{
		blasInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		// 재빌드가 아닌 업데이트 할 경우 아래의 source를 추가해야함
		blasBuildDesc.SourceAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
	}
	blasInputs.NumDescs = static_cast<uint32>(m_cacheGeometryDescs[m_currentID].size());
	blasInputs.pGeometryDescs = m_cacheGeometryDescs[m_currentID].data();

	blasBuildDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();
	blasBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();

	CommandList->BuildRaytracingAccelerationStructure(&blasBuildDesc, 0, nullptr);

	m_isDirty = false;	// 다시 수정하기 전까지 대기
	m_isBuilt = true;	// 이미 만들어졌다고 저장
}

void Ideal::DXRBottomLevelAccelerationStructure::BuildGeometries(std::vector<BLASGeometry>& Geometries)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
	geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDescTemplate.Triangles.IndexFormat = INDEX_FORMAT;
	geometryDescTemplate.Triangles.VertexFormat = VERTEX_FORMAT;
	m_geometryDescs.reserve(Geometries.size());

	m_geometryDescs.clear();
	for (BLASGeometry& geometry : Geometries)
	{
		D3D12_RAYTRACING_GEOMETRY_DESC& geometryDesc = geometryDescTemplate;


		// 2024.10.16 보류. 일관성이 보장 안된다. 예를 들어 같은 BLAS를 쓰고 있는데 어떤오브젝트를 처음에 생성하고 alphaclipping체크를 실행하면
		// 다음 인스턴스들은 무조건 투명이 된다. alpha clipping 체크를 안해도. 따라서 그냥 처음은 무조건 오파크로 만들고 alpha clipping 체크로 
		// 투명을 결정하게 한다.
		// 여기서 투명 오브젝트인지 검사하여 Flag를 결정한다.
		//bool IsAlphaClipping = geometry.Material.lock()->GetIsAlphaClipping();
		//if (IsAlphaClipping)
		//{
		//	// Any Hit Shader 실행
		//	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		//}
		//else
		//{
		//	// Any Hit Shader 무시
		//	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		//}
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDesc.Triangles.IndexBuffer = geometry.IndexBufferGPUAddress;
		geometryDesc.Triangles.IndexCount = geometry.IndexCount;
		geometryDesc.Triangles.VertexBuffer.StartAddress = geometry.VertexBufferGPUAddress;
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = geometry.VertexStrideInBytes;
		geometryDesc.Triangles.VertexCount = geometry.VertexCount;

		// 최종으로 만든 desc을 저장
		m_geometryDescs.push_back(geometryDesc);
	}

	m_geometries = Geometries;
}

//------------------------TLAS------------------------//

Ideal::DXRTopLevelAccelerationStructure::DXRTopLevelAccelerationStructure(const std::wstring& Name)
{
	m_name = Name;
}

Ideal::DXRTopLevelAccelerationStructure::~DXRTopLevelAccelerationStructure()
{

}

void Ideal::DXRTopLevelAccelerationStructure::Create(ComPtr<ID3D12Device5> Device, uint32 NumBLASInstanceDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags, std::shared_ptr<Ideal::DeferredDeleteManager> DeferredDeleteManager, bool AllowUpdate /*= false */)
{
	m_allowUpdate = AllowUpdate;
	m_buildFlags = BuildFlags;
	if (AllowUpdate)
	{
		m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	}

	// prebuild
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = m_buildFlags;
	topLevelInputs.NumDescs = NumBLASInstanceDescs;

	Device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &m_preBuildInfo);
	if (m_preBuildInfo.ResultDataMaxSizeInBytes <= 0)
	{
		MyMessageBoxW(L"Failed to prebuild AS");
	}

	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	//D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON;

	if (m_accelerationStructure == nullptr)
	{
		m_accelerationStructure = std::make_shared<Ideal::D3D12UAVBuffer>();
	}
	/*if (DeferredDeleteManager)
	{
		DeferredDeleteManager->AddD3D12ResourceToDelete(m_accelerationStructure->GetResource());
		m_accelerationStructure = std::make_shared<Ideal::D3D12UAVBuffer>();
	}*/
	m_accelerationStructure->Create(Device.Get(), m_preBuildInfo.ResultDataMaxSizeInBytes, initialResourceState, m_name.c_str());
}

void Ideal::DXRTopLevelAccelerationStructure::Build(ComPtr<ID3D12GraphicsCommandList4> CommandList, uint32 NumInstanceDesc, D3D12_GPU_VIRTUAL_ADDRESS InstanceDescsGPUAddress, ComPtr<ID3D12Resource> ScratchBuffer)
{

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& tlasInputs = tlasBuildDesc.Inputs;
	{
		tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		tlasInputs.Flags = m_buildFlags;
		if (m_isBuilt && m_allowUpdate)
		{
			// 2024.07.08 Update Flag일 때 들어오는 곳인데 매 프레임 재빌드를 하니 안들어온다.
			tlasInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

			// 샘플에서는 이 코드가 없지만 이거 안해주면 ERROR 남
			tlasBuildDesc.SourceAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
		}
		tlasInputs.NumDescs = NumInstanceDesc;
		tlasInputs.InstanceDescs = InstanceDescsGPUAddress;

		tlasBuildDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();
		tlasBuildDesc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
	}

	CommandList->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
	m_isDirty = false;
	m_isBuilt = true;
}