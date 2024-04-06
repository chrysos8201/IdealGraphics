#include "Common.hlsli"

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