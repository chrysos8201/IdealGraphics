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
    float4 color : COLOR;
};

PSInput VS(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position + offset;
    result.color = color;

    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    return input.color;
}
