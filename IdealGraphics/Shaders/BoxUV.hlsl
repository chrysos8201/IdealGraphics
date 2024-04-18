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

// cbuffer Transform : register(b0)
// {
//     float4x4 World;
//     float4x4 View;
//     float4x4 Proj;
//     float4x4 WorldInvTranspose;
// }

//
cbuffer TestOffset : register(b0)
{
    float4 offset;
    float4 padding[15];
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

PSInput VS(float4 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = position + offset;
    result.uv = uv;

    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.uv);
    return color;
}
