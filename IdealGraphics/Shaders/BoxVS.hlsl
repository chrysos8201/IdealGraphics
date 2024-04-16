//#include "Common.hlsli"

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
    float3 tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV : TEXCOORD;
};

VSOutput VS(VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4 localPos = float4(input.Pos, 1.f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    output.PosW = worldPos;
    output.PosH = projPos;
    output.NormalW = mul(WorldInvTranspose, float3(0.f,0.f,0.f));
    output.UV = input.UV;

    //float4 localPos = float4(input.pos, 1.f);
    //float4 worldPos = mul(World, localPos);
    //float4 viewPos = mul(View, worldPos);
    //float4 projPos = mul(Proj, viewPos);

    //output.svpos = projPos;
    //output.color = input.color;
    return output;
}