#ifndef CS_MODIFY_VERTEX_HLSL
#define CS_MODIFY_VERTEX_HLSL

struct BasicVertex
{
    float3 Position;
    float3 Normal;
    float2 TexCoord;
    float3 Tangent;
    float4 Color;
};


StructuredBuffer<BasicVertex> g_vertices : register(t0);

cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
}

cbuffer VertexCount : register(b1)
{
    uint g_vertexCount;
}

RWStructuredBuffer<BasicVertex> g_outputVertices : register(u0);

[numthreads(1024, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= g_vertexCount)
    {
        return;
    }
    BasicVertex inputVertex = g_vertices[DTid.x];
    BasicVertex outputVertex;
    float4 position = float4(inputVertex.Position, 1.f);
    position = mul(position, World);
    outputVertex.Position = position.xyz;

    float3 normal = normalize(mul(inputVertex.Normal, (float3x3)WorldInvTranspose));
    outputVertex.Normal = normal;

    float3 tangent = normalize(mul(inputVertex.Tangent, (float3x3)WorldInvTranspose));
    outputVertex.Tangent = tangent;

    outputVertex.TexCoord = inputVertex.TexCoord;
    outputVertex.Color = float4(0,0,0,1);
    g_outputVertices[DTid.x] = outputVertex;
    //SkinnedVertex inputVertex = g_vertices[DTid.x];
    //OutVertex outputVertex;
    //
    //float4x4 skinTransform = SkinTransform(inputVertex.BoneWeights, inputVertex.BoneIndices);
    //float4 position = float4(inputVertex.Position, 1.0f);
    //SkinVertex(position, inputVertex.Normal, skinTransform);
    //
    //outputVertex.Position = position.xyz;
    //outputVertex.Normal = inputVertex.Normal;
    //outputVertex.TexCoord = inputVertex.TexCoord;
    //outputVertex.Tangent = inputVertex.Tangent;
    //outputVertex.Color = float4(0.f,0.f,0.f,0.f);
    //g_outputVertices[DTid.x] = outputVertex;
}
#endif