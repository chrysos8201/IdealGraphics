
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

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION;
    float4 NormalW : NORMAL;
    float2 UV : TEXCOORD;
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
    output.NormalW = mul(WorldInvTranspose, float4(input.Normal, 0.f));
    output.UV = input.UV;

    return output;
}

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);

SamplerState sampler0 : register(s0);

float4 PS(VSOutput input) : SV_TARGET
{
    float4 color = diffuseTexture.Sample(sampler0, input.UV);
    float4 dirLight = float4(1.f,0.f,0.f,1.f);
    float4 value = dot(-dirLight, normalize(input.NormalW));
    color = color * value * Diffuse;
    return color;
}