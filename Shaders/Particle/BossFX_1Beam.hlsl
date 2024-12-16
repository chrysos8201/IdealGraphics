#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    // Pixel Shader
    float2 uv = input.UV;
    float2 tiling0 = float2(2, 2);
    float2 offset0 = float2(0, 0);
    
    //float timeOffset = g_Time; // 시간에 따른 오프셋
    uv += g_Time;
    // uv += float2(sin(timeOffset) * 0.1, cos(timeOffset) * 0.1); // 움직임

    
    Ideal_TilingAndOffset_float(uv, tiling0, offset0, uv);

    float4 colorTexture = ParticleTexture0.Sample(LinearWrapSampler, uv);
    float4 noiseTexture = ParticleTexture1.Sample(LinearWrapSampler, uv);
    
    // 노이즈 텍스처의 R 채널을 사용하여 컬러 텍스처의 알파 값을 조정
    float alpha = noiseTexture.r; // 흑백 텍스처의 R 채널을 알파로 사용
    float4 finalColor = colorTexture * alpha; // 노이즈를 이용한 혼합
    
    // cbuffer에서 설정된 색상 사용
    float4 startColor = g_startColor; // ParticleSystemData에서 색상 가져오기

    // 최종 색상에 초기 색상 적용
    finalColor.rgb *= startColor.rgb; // 색상 조정
    
    // 추가적인 효과 (예: 색상 변화)
    //float hueOffset = g_Time * 0.1; // 시간에 따라 색상 변화
    //Ideal_Hue_Degrees_float(finalColor.rgb, hueOffset, finalColor.rgb);

    return finalColor;
}


#endif
