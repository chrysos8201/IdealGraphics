#ifndef SCREEN_HLSL
#define SCREEN_HLSL

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct VSInput
{
    float4 Pos : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VS(VSInput Input)
{
    PSInput result;
    result.position = Input.Pos;
    result.uv = Input.TexCoord;
    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}

#endif