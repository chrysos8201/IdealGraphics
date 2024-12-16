#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    //float4 tex = ParticleTexture0.Sample(LinearWrapSampler, input.UV);
    float4 ret = float4(1, 1, 1, 1);
    return ret;
}

#endif
