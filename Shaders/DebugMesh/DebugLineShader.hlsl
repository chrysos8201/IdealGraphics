#ifndef DEFAULT_DEBUG_LINE_HLSL
#define DEFAULT_DEBUG_LINE_HLSL

#define MAX_INSTNACE_COUNT 3000
cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
};


cbuffer LineInfo : register(b1)
{
    float3 StartPos;
    float pad0;
    float3 EndPos;
    float pad1;
    float4 Color;
}

// cbuffer InstanceData : register(b1)
// {
//     LineInfo g_lineInfo[MAX_INSTNACE_COUNT];
// }

struct VSInput
{
    float3 Pos : POSITION;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

//VSOutput VSMain(VSInput input, UINT instanceID : SV_InstanceID)
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    //LineInfo currentLine = g_lineInfo[instanceID];
    //float3 worldPosition = lerp(currentLine.StartPos, currentLine.EndPos, input.Pos.x);
    float3 worldPosition = lerp(StartPos, EndPos, input.Pos.x);
    output.Pos = mul(float4(worldPosition, 1.f), ViewProj);
    output.Color = Color;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.Color;
}
#endif
