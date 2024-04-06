
cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float4x4 WorldInvTranspose;
}

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    //float3 tangent : TANGENT;
    //float4 color : COLOR;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV : TEXCOORD;
};