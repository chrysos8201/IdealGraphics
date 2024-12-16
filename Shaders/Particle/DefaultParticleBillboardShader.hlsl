#ifndef DEFAULT_PARTICLE_BILLBOARD_VS_HLSL
#define DEFAULT_PARTICLE_BILLBOARD_VS_HLSL

#include "ParticleCommon.hlsli" // necessary

VSParticleDrawOut VSMain(VSParticleInput input)
{
    VSParticleDrawOut output;
    //output.Pos = float3(0,0,0);
    //output.Pos = float3(input.ID * 10,0,0);
    output.Pos = g_bufPos[input.ID].Position.xyz;
    float rotationAngle = g_bufPos[input.ID].RotationAngle;
    output.Color = float4(rotationAngle,1,1,1);
    return output;
}

[maxvertexcount(4)]
void GSMain(point VSParticleDrawOut input[1], inout TriangleStream<VSOutput> SpriteStream)
{
    VSOutput output;
    // 빌보드를 만들겠다
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = eyePos - input[0].Pos;
    //look.y = 0.0f;  // y 축 정렬이므로 xz 평면에 투영
    look = normalize(look);
    float3 right = cross(up, look);
    up = cross(look, right);
    up = normalize(up);
    right = normalize(right);
    float halfWidth = 0.5f * g_ParticleSize.x; // * input[0].Size.x // TODO : 나중에 사이즈 추가 할 것
    float halfHeight = 0.5f * g_ParticleSize.y; // * input[0].Size.y // TODO : 나중에 사이즈 추가 할 것

    //////  Angle   //////
    //float rotationAngle = 1.57f; // TEMP
    float rotationAngle = input[0].Color.r;
    float cosAngle = cos(rotationAngle);
    float sinAngle = sin(rotationAngle);
    
    float2x2 rotationMatrix = float2x2(cosAngle, -sinAngle, sinAngle, cosAngle);
    
    // 빌보드의 네 꼭짓점을 계산(회전 적용)
    float2 localOffset[4] = {
        float2(halfWidth, -halfHeight),  // 오른쪽 아래
        float2(halfWidth, halfHeight),   // 오른쪽 위
        float2(-halfWidth, -halfHeight), // 왼쪽 아래
        float2(-halfWidth, halfHeight)   // 왼쪽 위
    };

    for(int i = 0; i < 4; i++)
    {
        // 회전 행렬 적용
        float2 rotatedOffset = mul(localOffset[i], rotationMatrix);
        float3 billboardPos = input[0].Pos + (rotatedOffset.x * right) + (rotatedOffset.y * up);
    
        output.PosH = mul(float4(billboardPos, 1.0), ViewProj);
        output.Pos = input[0].Pos.xyz;
        //output.Color = input[0].Color;
        // TEMP : 노말값 대충
        output.Normal = float3(1.f,0.f,0.f);
        output.UV = g_texcoords[i];
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();    // 삼각형 목록을 초기화
}

// TEST
float4 PSMain(VSOutput input) : SV_Target
{
    float4 ret;
    float2 animationUV = g_AnimationUV_Offset + (input.UV * g_AnimationUV_Scale); 
    //ret = ParticleTexture0.Sample(LinearWrapSampler, input.UV);
    ret = ParticleTexture0.Sample(LinearWrapSampler, animationUV);
    clip(ret.a - 0.1f);
    return ret;
}

#endif
