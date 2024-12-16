#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float2 scroll = float2(0.2, 0.0);
    float2 scrolledUV = input.UV + scroll * g_Time;
    // UV 타일링 및 오프셋
    float2 uvTiled;
    {
        float2 offset = scroll * g_Time;
        Ideal_TilingAndOffset_float(input.UV, float2(1.0, 1.0), offset, uvTiled);
    }   
    // Simple 노이즈 적용
    float noise;
    {
        Ideal_SimpleNoise_float(uvTiled, 50.0, noise);
    }
    // Step 함수 적용
    float stepData;
    {
        //float tempTime = 1 - g_Time;
        //float s = step(noise, tempTime);
        float s = step(noise, g_CustomData1.x);
        stepData = s;
    }

    float dis_amount = 0.2f;
    float gradientNoise;
    Ideal_GradientNoise_float(scrolledUV, 10, gradientNoise);
    
    //float2 outValue = lerp(input.UV, float2(gradientNoise, gradientNoise), dis_amount);
    float2 outValue = lerp(scrolledUV, gradientNoise, dis_amount);
    //float4 SampleTexture = ParticleTexture.Sample(LinearClampSampler, outValue);
    float4 SampleTexture = ParticleTexture0.Sample(LinearClampSampler, input.UV);
    float4 result = SampleTexture * stepData;

    float4 color = g_startColor;
    result *= color;
    result *= 4.0;
    return result;
}

#endif
