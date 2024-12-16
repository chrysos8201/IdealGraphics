
struct VertexIn
{
    float3 PosL : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH : SV_Position;
    float2 uv : TEXCOORD;
};

VertexOut main(VertexIn input)
{
    VertexOut output;
    output.uv = input.uv;
    output.PosH = float4(input.PosL, 1.f);
    
    return output;
}