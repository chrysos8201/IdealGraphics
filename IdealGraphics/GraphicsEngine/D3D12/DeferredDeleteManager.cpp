#include "DeferredDeleteManager.h"
#include "GraphicsEngine/public/IMeshObject.h"
#include "GraphicsEngine/D3D12/Raytracing/DXRAccelerationStructure.h"
#include "GraphicsEngine/D3D12/D3D12Texture.h"
#include "GraphicsEngine/Resource/IdealMaterial.h"

Ideal::DeferredDeleteManager::DeferredDeleteManager()
{

}

Ideal::DeferredDeleteManager::~DeferredDeleteManager()
{

}

void Ideal::DeferredDeleteManager::DeleteDeferredResources(uint32 CurrentContextIndex)
{
	DeleteD3D12Resource(CurrentContextIndex);
	DeleteTexture(CurrentContextIndex);
	DeleteMaterial(CurrentContextIndex);
	DeleteMeshObject(CurrentContextIndex);
	DeleteBLAS(CurrentContextIndex);
	DeleteTLAS(CurrentContextIndex);
	DeleteDescriptorHandle(CurrentContextIndex);
	m_currentContextIndex = CurrentContextIndex;
}
void Ideal::DeferredDeleteManager::AddD3D12ResourceToDelete(ComPtr<ID3D12Resource> D3D12Resource)
{
	m_resourcesToDelete[m_currentContextIndex].push_back(D3D12Resource);
}

void Ideal::DeferredDeleteManager::DeleteD3D12Resource(uint32 DeleteContextIndex)
{
	if (m_resourcesToDelete[DeleteContextIndex].size() > 0)
	{
		// 사실 ComPtr이라 Reset 안걸어줘도 됨
		for (auto& Resource : m_resourcesToDelete[DeleteContextIndex])
		{
			Resource.Reset();
		}
		m_resourcesToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddMeshObjectToDeferredDelete(std::shared_ptr<Ideal::IMeshObject>MeshObject)
{
	m_meshObjectsToDelete[m_currentContextIndex].push_back(MeshObject);
}

void Ideal::DeferredDeleteManager::DeleteMeshObject(uint32 DeleteContextIndex)
{
	if (m_meshObjectsToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_meshObjectsToDelete[DeleteContextIndex])
		{
			Resource.reset();
		}
		m_meshObjectsToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddBLASToDeferredDelete(std::shared_ptr<Ideal::DXRBottomLevelAccelerationStructure> BLAS)
{
	m_blasToDelete[m_currentContextIndex].push_back(BLAS);
}

void Ideal::DeferredDeleteManager::DeleteBLAS(uint32 DeleteContextIndex)
{
	if (m_blasToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_blasToDelete[DeleteContextIndex])
		{
			Resource->FreeMyHandle();
			Resource.reset();
		}
		m_blasToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddTLASToDeferredDelete(std::shared_ptr<Ideal::DXRTopLevelAccelerationStructure> TLAS)
{
	m_tlasToDelete[m_currentContextIndex].push_back(TLAS);
}

void Ideal::DeferredDeleteManager::DeleteTLAS(uint32 DeleteContextIndex)
{
	// TODO Delete
	if (m_tlasToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_tlasToDelete[DeleteContextIndex])
		{
			//Resource->FreeMyHandle();
			Resource.reset();
		}
		m_tlasToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddTextureToDeferredDelete(std::shared_ptr<Ideal::D3D12Texture> Texture)
{
	m_textureToDelete[m_currentContextIndex].push_back(Texture);
}

void Ideal::DeferredDeleteManager::DeleteTexture(uint32 DeleteContextIndex)
{
	if (m_textureToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_textureToDelete[DeleteContextIndex])
		{
			//Resource.reset();
			Resource->Free();
		}
		m_textureToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddMaterialToDefferedDelete(std::shared_ptr<Ideal::IdealMaterial> Material)
{
	m_materialToDelete[m_currentContextIndex].push_back(Material);
}

void Ideal::DeferredDeleteManager::DeleteMaterial(uint32 DeleteContextIndex)
{
	if (m_materialToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_materialToDelete[DeleteContextIndex])
		{
			// 현재 Free는 RayTracing의 핸들 값을 해제함.
			Resource->FreeInRayTracing();
			Resource.reset();
		}
		m_materialToDelete[DeleteContextIndex].clear();
	}
}

void Ideal::DeferredDeleteManager::AddDescriptorHandleToDeferredDelete(Ideal::D3D12DescriptorHandle2 DescriptorHandle)
{
	m_descriptorHandleToDelete[m_currentContextIndex].push_back(DescriptorHandle);
}

void Ideal::DeferredDeleteManager::DeleteDescriptorHandle(uint32 DeleteContextIndex)
{
	if (m_descriptorHandleToDelete[DeleteContextIndex].size() > 0)
	{
		for (auto& Resource : m_descriptorHandleToDelete[DeleteContextIndex])
		{
			Resource.Free();
		}
		m_descriptorHandleToDelete[DeleteContextIndex].clear();
	}
}
