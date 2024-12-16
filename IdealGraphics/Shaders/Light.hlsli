#ifndef _LIGHT_HLSLI
#define _LIGHT_HLSLI
#endif

#define MAX_POINT_LIGHT_NUM 1024
#define MAX_SPOT_LIGHT_NUM 1024

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
};

struct SpotLight
{
	float4 Color;
	float4 Direction;
	float3 Position;
	float SpotAngle;
	float Range;
	float Intensity;
};

cbuffer LightList : register(b1)	// temp b1
{
    DirectionalLight DirLight;
	PointLight PointLights[MAX_POINT_LIGHT_NUM];
	SpotLight SpotLights[MAX_SPOT_LIGHT_NUM];
};

struct Material
{
    float4 Diffuse;
};

float3 DirectLight(float lightIntensity, float3 lightColor, float3 toLight, float3 normal, float3 worldPos, float3 eyePos, float3 albedo)
{
    float3 toCamera = normalize(eyePos - worldPos);
    float3 diffuse = albedo;
    float3 NdotL = max(dot(normal, toLight), 0.f);
	
    return diffuse * NdotL * lightIntensity * lightColor.rgb;
}