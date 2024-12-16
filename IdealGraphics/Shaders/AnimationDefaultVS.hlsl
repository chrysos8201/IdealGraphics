// bone 연결시키기 위한 simple Mesh Shaderr

#define MAX_BONE_TRANSFORMS 350

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 tangent : TANGENT;
	float4 skinIndices	: BLEND_INDICES;
	float4 skinWeights	: BLEND_WEIGHTS;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION;
    float4 NormalW : NORMAL;
    float2 UV : TEXCOORD;
    float4  skinIndices	: BLEND_INDICES;
    float4 skinWeights	: BLEND_WEIGHTS;
};

cbuffer Material : register(b0)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
}

cbuffer Transform : register(b1)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float4x4 WorldInvTranspose;
}

cbuffer BoneBuffer : register(b2)
{
    matrix BoneTransforms[MAX_BONE_TRANSFORMS];
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
    normal = mul((float3x3)skinTransform, normal);
}

//uint BoneIndex;

Texture2D diffuseTexture : register(t0);
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
    output.NormalW = mul(WorldInvTranspose, float4(input.Normal, 0.f));
    output.UV = input.UV;
    output.skinIndices = input.skinIndices;
    output.skinWeights = input.skinWeights;
    return output;
}

float4 PS(VSOutput input) : SV_TARGET
{
    float4 color = diffuseTexture.Sample(sampler0, input.UV);
    float4 dirLight = float4(1.f,0.f,0.f,1.f);
    float4 value = dot(-dirLight, normalize(input.NormalW));
    color = color * value * Diffuse;
    return color;
}