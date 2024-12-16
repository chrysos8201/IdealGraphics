#ifndef DEFAULT_PARTICLE_MESH_BILLBOARD_VS_HLSL
#define DEFAULT_PARTICLE_MESH_BILLBOARD_VS_HLSL

#include "ParticleCommon.hlsli" // necessary

VSOutput Main(VSInput input)
{
     VSOutput output;
    
    float4 worldPos = mul(float4(input.Pos,1), World);
    output.Pos = worldPos.xyz;
    worldPos = mul(worldPos, ViewProj);
    output.PosH = worldPos;
    output.Normal = normalize(mul(input.Normal, (float3x3)World));
    output.UV = input.UV;
    
    return output;
}
#endif
