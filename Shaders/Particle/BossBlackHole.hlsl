#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    // UV 좌표
    float2 UV = input.UV;
 // 시간 기반 애니메이션
    float timeFactor = g_Time * 0.1f;// * g_CustomData2.w;
    UV += float2(timeFactor, 0); // UV 스크롤링

    float2 mainTexUV;
    Ideal_TilingAndOffset_float(UV, float2(1,2), float2(0,0), mainTexUV);
    // 기본 텍스처 샘플링
    float4 mainTexColor = ParticleTexture0.Sample(LinearWrapSampler, mainTexUV);
    mainTexColor.rgb *= g_startColor.rgb * 7.5; //factor
    //mainTexColor.rgb = mainTexColor.rgb / (mainTexColor.rgb + float3(1, 1, 1)); // 톤 매핑 예시 (Reinhard 톤 매핑)

    // 노말 텍스처 샘플링
    float4 normalTexColor = ParticleTexture1.Sample(LinearWrapSampler, UV);
    
    // 노말 벡터로 변환
    float3 normal = normalize(normalTexColor.rgb * 2.0 - 1.0); // [0,1] 범위를 [-1,1]로 변환

    // 노말 왜곡 처리
    {
        normal *= 0.5; // 노말 왜곡 강도 적용
    }

    // 노이즈 텍스처 추가
    float4 noiseTexColor = ParticleTexture2.Sample(LinearWrapSampler, UV); // 노이즈 텍스처 샘플링

   
    // 마스크 페이드 처리
    float maskFade = 1.0;
    //if (g_CustomData1.w > 0) // 마스크 페이드 활성화
    {
        float4 maskTexColor = ParticleTexture1.Sample(LinearWrapSampler, UV); // 마스크 텍스처 샘플링
        maskFade = maskTexColor.a; // 알파값으로 페이드 값 결정
    }

    // 최종 색상 계산
    float4 finalColor = mainTexColor * g_startColor * maskFade;

    // 알파 처리
    finalColor.a *= mainTexColor.a;

    // 리미트 조정
    finalColor = clamp(finalColor, 0.0, 1.0); // 색상 값 제한

    return finalColor;
}


#endif
