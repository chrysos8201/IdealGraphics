#pragma once
#include <dxgiformat.h>

#define USE_DEFRAG 1
#define USE_BUFFER_POOL 1
#define BUFFER_POOL_DEFRAG_MAX_COPY_SIZE_PER_FRAME				32*1024*1024

// 스태틱 매시 key로 캐싱해둘건지
#define CACHE_STATIC_MESH 1

#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#define BUFFER_POOL_DEFAULT_POOL_SIZE (32 * 1024 * 1024)
#define BUFFER_POOL_DEFAULT_POOL_MAX_ALLOC_SIZE	(16*1024*1024)
#define MIN_PLACED_RESOURCE_SIZE (64 * 1024)

#define RHI_POOL_ALLOCATOR_VALIDATE_LINK_LIST 1

// Test
//#define BeforeRefactor

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

//----USE UPLOAD COMMANDLIST CONTAINER----//
//#define USE_UPLOAD_CONTAINER
#define UPLOAD_CONTAINER_COUNT 8
#define UPLOAD_CONTAINER_LOOP_CHECK_COUNT 8
//------------Light-----------//
#define MAX_DIRECTIONAL_LIGHT_NUM 10
#define MAX_POINT_LIGHT_NUM 100
#define MAX_SPOT_LIGHT_NUM 100

// SWAP CHAIN & MAX PENDING

#define G_SWAP_CHAIN_NUM 4

// Raytracing Ray Recursion Depth
//#define G_MAX_RAY_RECURSION_DEPTH 2
#define G_MAX_RAY_RECURSION_DEPTH 5

// Index Format
#define INDEX_FORMAT DXGI_FORMAT_R32_UINT
#define VERTEX_FORMAT DXGI_FORMAT_R32G32B32_FLOAT

// Descriptor Heap Management
#define NUM_FIXED_DESCRIPTOR_HEAP 1	// shader table에 사용되는 고정된 Fixed Descriptor Heap의 개수
#define FIXED_DESCRIPTOR_HEAP_CBV_SRV_UAV 0	// 고정된 Descriptor Heap의 Index

// UI Canvas
namespace Ideal
{
	namespace RectRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				SRV_Sprite = 0,
				CBV_RectInfo,
				Count
			};
		}
	}

	namespace PostScreenRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				SRV_Scene = 0,
				Count
			};
		}
	}

	namespace ParticleSystemRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				CBV_Global = 0,
				CBV_Transform,
				CBV_ParticleSystemData,
				SRV_ParticlePosBuffer,
				SRV_ParticleTexture0,
				SRV_ParticleTexture1,
				SRV_ParticleTexture2,
				UAV_ParticlePosBuffer,	// Particle 포지션 위치계산을 적어놓기 위한 버퍼
				Count
			};
		}
	}

	namespace BasicRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				CBV_Global = 0,
				CBV_Transform,
				Count
			};
		}
	}

	namespace DebugMeshRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				CBV_Global = 0,
				CBV_Transform,
				CBV_Color,
				Count
			};
		}
	}

	namespace DebugLineRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				CBV_Global = 0,
				CBV_LineInfo,
				Count
			};
		}
	}
	
	namespace ModifyVertexBufferCSRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				SRV_Vertices = 0,
				CBV_Transform,
				CBV_VertexCount,
				UAV_OutputVertices,
				Count
			};
		}
	}

	namespace GenerateMipsCSRootSignature
	{
		namespace Slot
		{
			enum Enum
			{
				CBV_GenerateMipsInfo = 0,
				SRV_SourceMip,
				UAV_OutMip1,
				UAV_OutMip2,
				UAV_OutMip3,
				UAV_OutMip4,
				Count
			};
		}
	}
}
