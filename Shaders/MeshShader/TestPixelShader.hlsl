#ifndef TESTPIXELSHADER_HLSL
#define TESTPIXELSHADER_HLSL


struct MeshOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

//cbuffer Global : register(b0)
//{
//    float4x4 View;
//    float4x4 Proj;
//    float4x4 ViewProj;
//};

float4 PSMain(MeshOutput input) : SV_TARGET
{
    return float4(input.Color, 1);
}

#endif