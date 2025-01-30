#pragma once
#include <dxgiformat.h>

#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

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
#define G_MAX_RAY_RECURSION_DEPTH 2

// Index Format
#define INDEX_FORMAT DXGI_FORMAT_R32_UINT
#define VERTEX_FORMAT DXGI_FORMAT_R32G32B32_FLOAT

// Descriptor Heap Management
#define NUM_FIXED_DESCRIPTOR_HEAP 1	// shader table에 사용되는 고정된 Fixed Descriptor Heap의 개수
#define FIXED_DESCRIPTOR_HEAP_CBV_SRV_UAV 0	// 고정된 Descriptor Heap의 Index

/// Static Mesh Macro
// 2024.05.08
// Root Table Index Macro
#define STATIC_MESH_DESCRIPTOR_TABLE_INDEX_OBJ 1		// root parameter
#define	STATIC_MESH_DESCRIPTOR_INDEX_CBV_TRANSFORM 0	// b1 : transform

#define STATIC_MESH_DESCRIPTOR_TABLE_INDEX_MESH 2		// root parameter
#define	STATIC_MESH_DESCRIPTOR_INDEX_CBV_MATERIAL 0		// b2 : material
#define	STATIC_MESH_DESCRIPTOR_INDEX_SRV_DIFFUSE 1		// t0 : diffuse
#define	STATIC_MESH_DESCRIPTOR_INDEX_SRV_SPECULAR 2		// t1 : specular
#define	STATIC_MESH_DESCRIPTOR_INDEX_SRV_NORMAL 3		// t2 : normal

/// Skinned Mesh Macro
// 2024.05.08
// Root Table Index Macro
#define SKINNED_MESH_DESCRIPTOR_TABLE_INDEX_OBJ 1
#define SKINNED_MESH_DESCRIPTOR_INDEX_CBV_TRANSFORM 0	// b1 : transform
#define SKINNED_MESH_DESCRIPTOR_INDEX_CBV_BONE 1		// b2 : bone

#define SKINNED_MESH_DESCRIPTOR_TABLE_INDEX_MESH 2
#define SKINNED_MESH_DESCRIPTOR_INDEX_CBV_MATERIAL 0	// b3 : material
#define SKINNED_MESH_DESCRIPTOR_INDEX_SRV_DIFFUSE 1		// t0 : diffuse
#define SKINNED_MESH_DESCRIPTOR_INDEX_SRV_SPECULAR 2	// t1 : specular
#define SKINNED_MESH_DESCRIPTOR_INDEX_SRV_NORMAL 3		// t2 : normal

/// Global
// 2024.05.13
#define GLOBAL_DESCRIPTOR_TABLE_INDEX 0
#define GLOBAL_DESCRIPTOR_INDEX_CBV_GLOBAL 0 // b0 : global
#define GLOBAL_DESCRIPTOR_INDEX_CBV_LIGHTLIST 1 // b1 : lightList

/// Screen Quad
// 2024.05.15 Temp
#define SCREEN_DESCRIPTOR_TABLE_INDEX 1
#define SCREEN_DESCRIPTOR_INDEX_SRV_ALBEDO 0
#define SCREEN_DESCRIPTOR_INDEX_SRV_NORMAL 1
#define SCREEN_DESCRIPTOR_INDEX_SRV_POSH 2
#define SCREEN_DESCRIPTOR_INDEX_SRV_POSW 3
#define SCREEN_DESCRIPTOR_INDEX_SRV_Depth 4

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