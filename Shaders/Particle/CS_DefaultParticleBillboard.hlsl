#ifndef CS_DEFAULT_PARTICLE_Billboard_HLSL
#define CS_DEFAULT_PARTICLE_Billboard_HLSL

#include "ParticleCommon.hlsli" 

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    // Particle MaxCount
    if(index >= g_MaxParticles)
    {
        return;
    }
    if(g_RWBufferPos[index].DelayTime >= g_currentTime)
    {
        return;
    }
    // TODO UAV 연결
    // 포지션, 방향, 속력 계산
    float3 NextPosition = g_RWBufferPos[index].Position.xyz;
    NextPosition += (g_RWBufferPos[index].Direction * g_RWBufferPos[index].Speed * g_DeltaTime);
    //NextPosition += (g_RWBufferPos[index].Direction * g_RWBufferPos[index].Speed);// * g_DeltaTime);
    g_RWBufferPos[index].Position = float4(NextPosition, 1.f);
    //g_RWBufferPos[index].Position = float4(1.f,1.f,1.f,1.f);
}

#endif
