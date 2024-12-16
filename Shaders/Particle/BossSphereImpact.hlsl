#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{  // 기본 색상
     // 기본 색상
    float4 baseColor = ParticleTexture0.Sample(LinearWrapSampler, input.UV);
    
    baseColor.rgb *= g_startColor.rgb;
    baseColor.rgb = baseColor.rgb / (baseColor.rgb + float3(1, 1, 1)); // 톤 매핑 예시 (Reinhard 톤 매핑)

    baseColor.a *= g_startColor.a;

    // Color Factor
    baseColor.rgb *= g_CustomData1.z;

    // 시간에 따른 애니메이션 조정 (Texture Animate)
    float2 animatedUV = input.UV;
    float2 animatedUVTO;
    Ideal_TilingAndOffset_float(input.UV, float2(g_CustomData1.y, 2), float2(0, 0), animatedUVTO);    // offet customaData에서 불러옴
    animatedUVTO += g_Time * g_CustomData1.x; // 애니메이션 속도 조정
    baseColor *= ParticleTexture0.Sample(LinearWrapSampler, animatedUVTO); // 텍스처 애니메이션 적용

    // 노멀 왜곡 (Normal Distortion)
    float2 normalUV;
    Ideal_TilingAndOffset_float(input.UV, float2(2, 2), float2(0, 0), normalUV);
    float3 normal = ParticleTexture1.Sample(LinearWrapSampler, normalUV).rgb; // 노멀 맵 사용
    normal = normalize(normal * 2.0 - 1.0); // [-1, 1] 범위로 변환

    // 마스크 페이드 (Mask Fade)
    float2 maskUV;
    Ideal_TilingAndOffset_float(input.UV, float2(1, -1), float2(0, 0), maskUV);
    float4 maskAlpha = ParticleTexture2.Sample(LinearWrapSampler, maskUV); // 마스크 텍스처 사용

    baseColor *= maskAlpha;
    
    // 최종 색상 반환
    return baseColor;
}


#endif
