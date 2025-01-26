#ifndef DEFAULT_TERRAIN_HLSL
#define DEFAULT_TERRAIN_HLSL

cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
};

cbuffer Transform : register(b1)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
}

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    
    float4 worldPos = mul(float4(input.Pos,1), World);
    worldPos = mul(worldPos, ViewProj);
    output.Pos = worldPos;//.xyz;

    output.Color = input.color;

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.Color;
}
#endif
