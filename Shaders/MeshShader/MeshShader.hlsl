#ifndef MESHSHADER_HLSL
#define MESHSHADER_HLSL

struct MeshOutput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
};

struct BasicVertex
{
    float3 Position;
    float3 Normal;
    float2 UV;
    float3 Tangent;
    float4 Color;
};

struct Meshlet
{
    uint VertexOffset;
    uint TriangleOffset;
    uint VertexCount;
    uint TriangleCount;
};

StructuredBuffer<BasicVertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
ByteAddressBuffer MeshletTriangles : register(t2);
StructuredBuffer<uint> VertexIndices : register(t3);


/////
// Data Loaders

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet m, uint index)
{
    return UnpackPrimitive(VertexIndices[m.TriangleOffset + index]);
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
    localIndex = m.VertexOffset + localIndex;

    //if (MeshInfo.IndexBytes == 4) // 32-bit Vertex Indices
    //{
    //    return MeshletTriangles.Load(localIndex * 4);
    //}
    //else // 16-bit Vertex Indices
    // 인덱스가 
    {
        // Byte address must be 4-byte aligned.
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
        uint indexPair = MeshletTriangles.Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

MeshOutput GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    BasicVertex v = Vertices[vertexIndex];

    MeshOutput vout;
    vout.Position = mul(float4(v.Position, 1), ViewProj);   
    vout.Color = meshletIndex;

    return vout;
}

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void MSMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID, 
    out indices uint3 tris[128],
    out vertices MeshOutput verts[64]
)
{
    Meshlet m = Meshlets[gid];
    SetMeshOutputCounts(m.VertexCount, m.TriangleCount);
    // meshlet 내에서 최대 삼각형 개수를 넘어가지 않고 계산...인데..
    if(gtid < m.TriangleCount)
    {
        tris[gtid] = GetPrimitive(m, gtid);
    }
    // meshlet 내에서 최대 개수를 넘어가지 않고 계산. ~128?
    if(gtid < m.VertexCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid);
        verts[gtid] = GetVertexAttributes(gid, vertexIndex);
    }
}

float4 PSMain(MeshOutput input) : SV_TARGET
{
    //return float4(input.Color, 1);
    return float4(1,1,1,1);
}


#endif