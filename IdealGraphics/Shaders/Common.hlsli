
cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
}

struct VSInput
{
    float3 pos : POSITION;
    //float3 normal : NORMAL;
    //float2 uv : TEXCOORD;
    //float3 tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 color : COLOR;
};