#ifndef DEFAULT_TERRAIN_HLSL
#define DEFAULT_TERRAIN_HLSL

cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
};

cbuffer Transform : register(b1)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
}

struct VSInput
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    
    //float4 worldPos = mul(float4(input.Pos,1), World);
    //worldPos = mul(worldPos, ViewProj);
    //output.Pos = worldPos;//.xyz;
    //
    //output.Color = input.color;

    output.Pos = float4(input.Pos, 1);
    output.Color = input.color;
    
    return output;
}

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VSOutput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    float3 centerL = 0.25f * ((float3) (patch[0].Pos + patch[1].Pos + patch[2].Pos + patch[3].Pos));
    float3 centerW = mul(float4(centerL, 1.f), World).xyz;
    
    float d = distance(centerW, -float3(View._41, View._42, View._43));
    
    const float d0 = 20.f;
    const float d1 = 100.f;
    float tess = 64.f * saturate((d1 - d) / (d1 - d0));
    
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    pt.EdgeTess[3] = tess;
    
    pt.InsideTess[0] = tess;
    pt.InsideTess[1] = tess;
    
    return pt;
}

struct HullOut
{
    float3 PosL : POSITIONT;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.f)]
HullOut HSMain(InputPatch<VSOutput,4> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosL = (float3)p[i].Pos;
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

[domain("quad")]
DomainOut DSMain(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    // Bilinear interpolation
    float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
    float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
    float3 p = lerp(v1, v2, uv.y);
    
    p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));

    //dout.PosH = mul(float4(p, 1.f), mul(ViewProj, World));
    dout.PosH = mul(float4(p, 1.f), mul(World, ViewProj));
    return dout;
}

float4 PSMain(DomainOut input) : SV_TARGET
{
    return float4(1.f, 1.f, 1.f, 1.f);
}
#endif
