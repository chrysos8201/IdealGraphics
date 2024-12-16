#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 output;

    // 카메라 위치를 View 행렬에서 계산
    float4 cameraPosInWorld = float4(eyePos, 1); // View 행렬에서 카메라 위치 추출
    float3 viewDir = normalize(cameraPosInWorld.xyz - input.Pos); // 카메라 방향

    // 법선과 카메라 방향 간의 각도 계산
    float rim = saturate(dot(input.Normal, viewDir)); // 내적을 통해 각도 계산
    rim = pow(1 - rim, 2);

    // 림라이트 효과를 위한 색상 보간
    output = rim * g_startColor;

    output.a = 1; // 알파 값 설정 (불투명)
    
    return output;
}


#endif
