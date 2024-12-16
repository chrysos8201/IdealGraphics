#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 colorTexture = ParticleTexture0.Sample(LinearWrapSampler, input.UV);
    
    colorTexture *= g_startColor;    
    clip(colorTexture.a - 0.1f);
    return colorTexture ;
}


#endif
