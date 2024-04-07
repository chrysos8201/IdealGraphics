#include "Common.hlsli"

float4 PS(VSOutput input) : SV_TARGET
{
    //return float4(1.f,1.f,1.f,1.f);
    return float4(input.UV.xy, 1, 1);
    //return input.color;
}