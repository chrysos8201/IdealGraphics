#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    float2 maskUV = input.UV;

    // 시간에 따른 애니메이션을 위한 UV 변환
    float scrollSpeed = 2.5; // 스크롤 속도 설정
    uv.y += g_Time * scrollSpeed; // 화염 텍스처의 Y축 스크롤

    // 마스크 UV를 앞으로 모이는 효과
    maskUV.y -= g_Time * 0.5; // 마스크가 위쪽으로 이동하여 앞으로 모이는 느낌을 주기 위한 조정

    // 텍스처 샘플링
    float4 maskColor = ParticleTexture0.Sample(LinearWrapSampler, maskUV); // 마스크 텍스처
    float4 fireColor = ParticleTexture1.Sample(LinearWrapSampler, uv); // 화염 텍스처
    float4 normalMapColor = ParticleTexture2.Sample(LinearWrapSampler, uv); // 노멀 맵

    // 알파값 반전 처리
    float invertedAlpha = 1.0 - maskColor.a; 

    // 혼합 효과: 화염 색상에 반전된 알파값을 곱해줌
    float4 outputColor = fireColor * invertedAlpha; 
    outputColor.rgb *= maskColor.rgb; // 마스크 색상으로 화염 색상 조정

    // 노멀 맵을 사용하여 세부 사항 추가
    outputColor.rgb += normalMapColor.rgb * 0.2; // 노멀 맵의 영향을 추가

    // 결과 색상 반환
    return outputColor;
}


#endif
