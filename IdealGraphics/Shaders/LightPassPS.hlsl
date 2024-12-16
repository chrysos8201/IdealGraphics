
#define MAX_POINT_LIGHT_NUM 16
#define MAX_SPOT_LIGHT_NUM 16

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Intensity;
};

struct PointLight
{
	float4 Color;
	float3 Position;
	float Range;
	float Intensity;
    float3 pad;
};

struct SpotLight
{
	float4 Color;
	float4 Direction;
	float3 Position;
	float SpotAngle;
	float Range;
	float Intensity;
    float2 pad;
};

struct VertexOut
{
	float4 PosH : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer Global : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewProj;
    float3 eyePos;
}

float Attenuate(float3 position, float range, float3 worldPos)
{
    float dist = distance(position, worldPos);
	float att = saturate(1.0f - (dist * dist / (range * range)));
	return att * att;
    

    //float dist = distance(position, worldPos);
    //float numer = dist / range;
    //numer = numer * numer;
    //numer = numer * numer;
    //numer = saturate(1 - numer);
    //numer = numer * numer;
    //float denom = dist * dist + 1;
    //return (numer / denom);
}

void ComputeDirectionalLight(DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.f,0.f,0.f,0.f);
    diffuse = float4(0.f,0.f,0.f,0.f);
    specular = float4(0.f,0.f,0.f,0.f);

    float3 lightVec = -L.Direction;
    
    ambient = L.AmbientColor;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if(diffuseFactor > 0.f)
    {
        float3 v = reflect(-lightVec, normal);
        //float specFactor = pos(max(dot(v, toEye), 0.f) mat))

        // temp ( 0.4f 를 Mat.diffuse로 바꿀 것)
        diffuse = diffuseFactor * float4(0.4f, 0.4f, 0.4f, 1.f) * L.DiffuseColor;
        // Specular 계산
    }
}

void ComputePointLight(PointLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.f,0.f,0.f,0.f);
    diffuse = float4(0.f,0.f,0.f,0.f);
    specular = float4(0.f,0.f,0.f,0.f);

    float3 lightVec = L.Position - pos;
    float d = length(lightVec);

    if(d > L.Range)
        return;

    lightVec /= d;

    //ambient = L.Color
    float diffuseFactor = dot(lightVec, normal);

    [flatten]
    if(diffuseFactor > 0.f)
    {
        float3 v = reflect(-lightVec, normal); 
        //float specFactor = pow(max(dot(v, toEye), 0.f), mat.Specular.w));
        //diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        diffuse = diffuseFactor * float4(0.4f,0.4f,0.4f,1.f) * L.Color;
        //spec = specFactor * mat.Specular * L.
    
    }

    float att = Attenuate(L.Position, L.Range, pos);
    float lightIntensity = L.Intensity * att;
    diffuse *= lightIntensity;
    specular *= lightIntensity;
}



cbuffer LightList : register(b1)
{
    int PointLightNum;
    int SpotLightNum;
    float2 pad;
    DirectionalLight DirLight;
	PointLight PointLights[MAX_POINT_LIGHT_NUM];
	SpotLight SpotLights[MAX_SPOT_LIGHT_NUM];
};

Texture2D gAlbedoTexture : register(t0);
Texture2D gNormalTexture : register(t1);
Texture2D gPosH: register(t2);
Texture2D gPosW: register(t3);

SamplerState basicSampler : register(s0);

float4 main(VertexOut input) : SV_TARGET
{
	float4 albedo = gAlbedoTexture.Sample(basicSampler, input.uv);
	float4 normalOri = gNormalTexture.Sample(basicSampler, input.uv);
    float4 posH = gPosH.Sample(basicSampler, input.uv);
    float4 posW = gPosW.Sample(basicSampler, input.uv);
    
    if(normalOri.w >= 1.f)
    {
        return albedo;
    }
	float3 normal = normalOri.xyz;
	normalize(normal);
    
    float3 toEye = eyePos - posW.xyz;
    

    float3 lightVec = -DirLight.Direction;
    normalize(lightVec);
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A, D, S;
    //---Directional Light---//
    ComputeDirectionalLight(DirLight, normal, toEye, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    for(int i = 0; i < PointLightNum; ++i)
    {
        ComputePointLight(PointLights[i], posW.xyz, normal, toEye, A,D,S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    float4 litColor = albedo * (ambient + diffuse);
    litColor.a = 1.f;
    return litColor;
}