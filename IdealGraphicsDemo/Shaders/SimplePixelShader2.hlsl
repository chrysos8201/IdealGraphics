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

//#include "SimpleVertexShader.hlsl"
struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};


float4 main(PSInput input) : SV_TARGET
{
    return float4(1.f,1.f,1.f, 1.0f);
}