#ifndef MESHSHADER_HLSL
#define MESHSHADER_HLSL

struct MeshOutput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

//cbuffer Global : register(b0)
//{
//    float4x4 View;
//    float4x4 Proj;
//    float4x4 ViewProj;
//};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MSMain(out indices uint3 triangles[1], out vertices MeshOutput vertices[3])
{
    SetMeshOutputCounts(3, 1);
    triangles[0] = uint3(0, 1, 2);

    vertices[0].Position = float4(-0.5, 0.5, 0.0, 1.0);
    vertices[0].Color = float3(1.0, 0.0, 0.0);

    vertices[1].Position = float4(0.5, 0.5, 0.0, 1.0);
    vertices[1].Color = float3(0.0, 1.0, 0.0);

    vertices[2].Position = float4(0.0, -0.5, 0.0, 1.0);
    vertices[2].Color = float3(0.0, 0.0, 1.0);
}

float4 PSMain(MeshOutput input) : SV_TARGET
{
    return float4(input.Color, 1);
}


#endif