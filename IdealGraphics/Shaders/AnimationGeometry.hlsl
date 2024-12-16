// bone 연결시키기 위한 simple Mesh Shaderr

#define MAX_BONE_TRANSFORMS 250

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    float4 skinIndices : BLEND_INDICES;
    float4 skinWeights : BLEND_WEIGHTS;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    float4 skinIndices : BLEND_INDICES;
    float4 skinWeights : BLEND_WEIGHTS;
};

struct PSOutput
{
// Test
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 PosH : SV_Target2;
    float4 PosW : SV_Target3;
};

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

cbuffer BoneBuffer : register(b2)
{
    matrix BoneTransforms[MAX_BONE_TRANSFORMS];
}

cbuffer Material : register(b3)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
}

float4x4 SkinTransform(float4 weights, float4 boneIndices)
{
    float4x4 skinTransform =
            BoneTransforms[boneIndices.x] * weights.x +
            BoneTransforms[boneIndices.y] * weights.y +
            BoneTransforms[boneIndices.z] * weights.z +
            BoneTransforms[boneIndices.w] * weights.w;
    return skinTransform;
}

void SkinVertex(inout float4 position, inout float3 normal, float4x4 skinTransform)
{
    //position = mul(position, skinTransform);
    position = mul(skinTransform, position);
    normal = mul((float3x3) skinTransform, normal);
}

//uint BoneIndex;

Texture2D albedoTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
SamplerState sampler0 : register(s0);

VSOutput VS(VSInput input)
{
    VSOutput output;

    float4x4 skinTransform = SkinTransform(input.skinWeights, input.skinIndices);
    float4 position = float4(input.Pos, 1.0f);
    
    SkinVertex(position, input.Normal, skinTransform);


    //float4 localPos = float4(input.Pos, 1.f);
    float4 localPos = position;
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    output.PosW = worldPos;
    output.PosH = projPos;
    output.Normal = normalize(mul((float3x3) World, input.Normal));
    output.Tangent = normalize(mul((float3x3) World, input.Tangent));
    output.UV = input.UV;
    output.skinIndices = input.skinIndices;
    output.skinWeights = input.skinWeights;
    return output;
}

float3 CalculateNormalFromMap(float3 normalFromTexture, float3 normal, float3 tangent)
{
    float3 unpackedNormal = normalFromTexture * 2.f - 1.f;
    float3 N = normal;
    //float3 T = normalize(tangent - N * dot(tangent, N));
    float3 T = normalize(tangent - dot(N, tangent) * N);
    //float3 B = cross(N, T);
    float3 B = cross(T, N);
//    float3x3 TBN = float3x3(T, B, N);
    float3x3 TBN = float3x3(N, B, T);
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
    
    //float3 worldNormal = CalculateNormalFromMap(normalFromTexture, normalize(input.Normal), input.Tangent);
    //float4x4 viewTrans = transpose(View);
    //psOutput.Normal = float4(normalize(mul(worldNormal, (float3x3) viewTrans)), 1.f);
    //psOutput.Normal = float4(input.Normal, 1.f);

    psOutput.Normal.xyz = input.Normal;
    psOutput.Normal.w = input.PosH.z;
    //psOutput.Normal = mul(WorldInvTranspose, float4(input.Normal, 1.f));
    psOutput.PosW = input.PosW;
    
    //float DepthZ = input.PosH.z;
    //psOutput.PosH = float4(DepthZ, DepthZ, DepthZ, 1.f);
    psOutput.PosH = input.PosH;
    return psOutput;
}