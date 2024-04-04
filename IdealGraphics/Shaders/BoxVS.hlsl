#include "Common.hlsli"

void VS(float3 iPosL : POSITION, float4 iColor : COLOR, out float4 oPosH : SV_Position, out float4 oColor : COLOR)
{
    oPosH = mul(float4(iPosL, 1.0f), gWorldViewProj);
    
    oColor = iColor;
}