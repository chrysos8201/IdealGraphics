//#include "Common.hlsli"

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 UV : TEXCOORD;
};

float4 PS(VSOutput input) : SV_TARGET
{
    //return float4(1.f,1.f,1.f,1.f);
    return float4(input.UV.xy, 1, 1);
    //return input.color;
}