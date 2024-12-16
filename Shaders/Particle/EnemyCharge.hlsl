#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 color = g_startColor;
    float4 tex = ParticleTexture0.Sample(LinearWrapSampler, input.UV);
    float2 newUV;
    Ideal_TilingAndOffset_float(input.UV, float2(2,10), float2(g_CustomData1.x, g_CustomData1.x), newUV);
    float4 tex1 = ParticleTexture1.Sample(LinearWrapSampler, newUV);
    color *= tex1;
    return color;
}

#endif
