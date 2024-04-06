#include "Common.hlsli"

float4 PS(VSOutput input) : SV_TARGET
{
    return float4(1.f,1.f,1.f,1.f);
    //return input.color;
}