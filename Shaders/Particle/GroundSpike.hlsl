#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 result = g_startColor;
    float remapValue;
    float4 noiseTex = ParticleTexture0.Sample(LinearClampSampler, input.UV);
    float mask = ParticleTexture1.Sample(LinearClampSampler, input.UV).r;
    float dissolveAmount = 0.5;
    float adjustedMask = mask * (1 - 0.1) + 0.1/2;
    float alpha = saturate((adjustedMask - g_CustomData1.x) / 0.1 + 0.5);
    result *= noiseTex;
    result *= alpha;
    return result;
}


#endif
