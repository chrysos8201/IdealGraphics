//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct VSInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer PerDraw : register(b0)
{
    float4x4 worldViewProjection;
};

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

PSInput VS(VSInput input) 
{
    PSInput result;

    result.position = mul(input.position, worldViewProjection);
    result.uv = input.uv;

    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.uv);
    return color;
}
