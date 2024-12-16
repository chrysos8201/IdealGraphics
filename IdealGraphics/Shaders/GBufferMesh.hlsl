#ifndef _GBUFFER_SHADER
#define _GBUFFER_SHADER
#endif
//https://darkcatgame.tistory.com/11
cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
}

cbuffer Transform : register(b1)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
}

cbuffer Material : register(b2)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
}

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
};

struct PSOutput
{
// Test
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 PosH : SV_Target2;
    float4 PosW : SV_Target3;
};

VSOutput VS(VSInput input)
{
    VSOutput output;

    float4 localPos = float4(input.Pos, 1.f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
   
    output.PosW = worldPos;
    output.PosH = projPos;
    //output.NormalW = mul(WorldInvTranspose, float4(input.Normal, 0.f));
    output.Normal = normalize(mul((float3x3)World, input.Normal));
    output.Tangent = normalize(mul((float3x3)World, input.Tangent));
    output.UV = input.UV;

    return output;
}

Texture2D albedoTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
SamplerState sampler0 : register(s0);

float3 CalculateNormalFromMap(float3 normalFromTexture, float3 normal, float3 tangent)
{
    float3 unpackedNormal = normalFromTexture * 2.f - 1.f;
    float3 N = normal;
    float3 T = normalize(tangent - N * dot(tangent, N));
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T,B,N);
    return normalize(mul(TBN, unpackedNormal));
}

void ComputeNormalMapping(inout float3 normal, float3 tangent, float2 uv)
{
    float4 map = normalTexture.Sample(sampler0, uv);
    if(any(map.rgb) == false)
        return;

    float3 N = normalize(normal);
    float3 T = normalize(tangent);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    
    float3 tangentSpaceNormal = (map.rgb * 2.f - 1.f);
    float3 worldNormal = mul(tangentSpaceNormal, TBN);

    normal = worldNormal;
}

PSOutput PS(VSOutput input)
{

    ComputeNormalMapping(input.Normal, input.Tangent, input.UV);
    PSOutput psOutput;
    float4 color = albedoTexture.Sample(sampler0, input.UV);
    float3 normalFromTexture = normalTexture.Sample(sampler0, input.UV).rgb;
    psOutput.Albedo = color;
    //psOutput.Normal = float4(CalculateNormalFromMap(normalFromTexture, normalize(input.Normal), input.Tangent), 1.f);
    psOutput.Normal.xyz = input.Normal;
    psOutput.Normal.w = input.PosH.z;
    psOutput.PosW = input.PosW;
    
    //float DepthZ = input.PosH.z;
    //psOutput.PosH = float4(DepthZ, DepthZ, DepthZ, 1.f);
    psOutput.PosH = input.PosH;
    return psOutput;
}