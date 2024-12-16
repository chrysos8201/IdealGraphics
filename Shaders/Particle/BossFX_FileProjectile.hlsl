#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    float2 maskUV = input.UV;

    // 시간에 따른 애니메이션을 위한 UV 변환
    float scrollSpeed = 2.5; // 스크롤 속도 설정
    //float scrollSpeed = 5; // 스크롤 속도 설정
    Ideal_TilingAndOffset_float(maskUV, float2(1,-1), float2(0,0), maskUV);
    uv.y += g_Time * scrollSpeed; // 시간에 따라 Y축 방향으로 스크롤
    
    // 마스크 텍스처와 화염 텍스처 샘플링
    float4 maskColor = ParticleTexture0.Sample(LinearWrapSampler, maskUV);
    float4 fireColor = ParticleTexture1.Sample(LinearWrapSampler, uv);
    
    // 혼합 효과: 마스크 색상에 따라 화염 색상을 조정
    float4 outputColor = g_startColor * fireColor * maskColor.a; // 알파값을 사용하여 혼합
    outputColor.rgb *= maskColor.rgb;
    // 결과 색상 반환
    //float4 finalColor = float4(outputColor.rgb, 1);
    //finalColor.rgb *= g_startColor;
    //return finalColor;
    return outputColor;
}


#endif
