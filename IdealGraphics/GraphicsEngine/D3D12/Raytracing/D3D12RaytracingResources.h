#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12Resource.h"
#include <DirectXMath.h>

namespace Ideal
{
	inline UINT Align(UINT size, UINT alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}

	class DXRShaderRecord
	{
	public:
		DXRShaderRecord(void* pShaderIdentifier, uint32 shaderIdentifierSize)
			: shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
		{

		}

		DXRShaderRecord(
			void* pShaderIdentifier, uint32 shaderIdentifierSize,
			void* pLocalRootArguments, uint32 localRootArgumentsSize
		) : shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
			localRootArguments(pLocalRootArguments, localRootArgumentsSize)
		{

		}

		void CopyTo(void* dest) const
		{
			uint8_t* byteDest = static_cast<uint8_t*>(dest);
			memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
			if (localRootArguments.ptr)
			{
				memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
			}
		}

		struct PointerWithSize
		{
			void* ptr;
			uint32 size;

			PointerWithSize() : ptr(nullptr), size(0) {}
			PointerWithSize(void* _ptr, uint32 _size) : ptr(_ptr), size(_size) {};
		};
		PointerWithSize shaderIdentifier;
		PointerWithSize localRootArguments;
	};

	class DXRShaderTable : public D3D12UploadBuffer
	{
	public:
		DXRShaderTable(ID3D12Device* device, uint32 numShaderRecords, uint32 shaderRecordSize, LPCWSTR resourceName = nullptr)
		{
			m_shaderRecordSize = Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
			m_shaderRecords.reserve(numShaderRecords);
			uint32 bufferSize = numShaderRecords * m_shaderRecordSize;
			Create(device, bufferSize);
			m_mappedShaderRecords = (uint8_t*)MapCpuWriteOnly();
			m_resource->SetName(resourceName);
		}

		void push_back(const DXRShaderRecord& shaderRecord)
		{
			Check(m_shaderRecords.size() < m_shaderRecords.capacity());
			m_shaderRecords.push_back(shaderRecord);
			shaderRecord.CopyTo(m_mappedShaderRecords);
			m_mappedShaderRecords += m_shaderRecordSize;
		}

		uint32 GetShaderRecordSize() { return m_shaderRecordSize; }
		uint32 GetNumShaderRecords() { return static_cast<uint32>(m_shaderRecords.size()); }

	private:
		uint8_t* m_mappedShaderRecords;
		uint32 m_shaderRecordSize;

		// Debug support
		std::vector<DXRShaderRecord> m_shaderRecords;
	};

	// add utils
	struct DXRInstanceDesc : public D3D12_RAYTRACING_INSTANCE_DESC
	{
		void SetTransform(const Matrix& NewTransform)
		{
			XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(D3D12_RAYTRACING_INSTANCE_DESC::Transform), NewTransform);
		}
		Matrix const& GetTransform()
		{
			DirectX::XMMATRIX transform =XMLoadFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(D3D12_RAYTRACING_INSTANCE_DESC::Transform));
			Matrix ret = transform;
			return ret;
		}
	};
}