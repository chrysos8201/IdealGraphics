#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12Definitions.h"

#define MAX_BONE_TRANSFORMS 600
#define MAX_MODEL_KEYFRAMES 600


struct SceneConstantBuffer
{
	Matrix View;
	Matrix Proj;
	Matrix ProjToWorld;
	DirectX::XMVECTOR CameraPos;

	uint32 maxRadianceRayRecursionDepth;
	uint32 maxShadowRayRecursionDepth;
	float nearZ;
	float farZ;

	float resolutionX;
	float resolutionY;

	float FOV;
	float AmbientIntensity;
};

struct CB_MaterialInfo
{
	uint32 bUseDiffuseMap;
	uint32 bUseNormalMap;
	uint32 bUseMetallicMap;
	uint32 bUseRoughnessMap;

	float metallicFactor;
	float roughnessFactor;

	Vector2 tiling;
	Vector2 offset;

	uint32 bIsTransmissive;
	uint32 Layer = 0;

	float pad0 = 0;
};

struct CB_Bone
{
	Matrix transforms[MAX_BONE_TRANSFORMS];
};

struct CB_Color
{
	Color color;
};

struct CB_Transform
{
	Matrix World;
	Matrix WorldInvTranspose;
};

struct CB_Material
{
	Color Ambient;
	Color Diffuse;
	Color Specular;
	Color Emissive;
};

struct CB_Global
{
	Matrix View;
	Matrix Proj;
	Matrix ViewProj;
	Vector3 eyePos;
};

struct CB_DebugLine
{
	Vector3 startPos;
	float pad0;
	Vector3 endPos;
	float pad1;
	Color color;
};

struct CB_GenerateMipsInfo
{
	uint32 SrcMipLevel;
	uint32 NumMipLevels;
	uint32 SrcDimension;
	bool IsSRGB;
	Vector2 TexelSize;
};

struct PointLight
{
	Color Color;
	Vector3 Position;
	float Range;
	float Intensity;
	uint32 IsShadowCasting;
	uint32 Layer;
	float pad2;
};

struct DirectionalLight
{
	Color AmbientColor;
	Color DiffuseColor;
	Vector3 Direction;
	float Intensity;
};

struct SpotLight
{
	Color Color;
	Vector3 Direction;
	float SpotAngle;
	Vector3 Position;
	float Range;	
	float Intensity;
	float Softness;
	float pad1;
	float pad2;
};

struct CB_LightList
{
	int32 DirLightNum;
	int32 PointLightNum;
	int32 SpotLightNum;
	float pad0;

	DirectionalLight DirLights[MAX_DIRECTIONAL_LIGHT_NUM];
	PointLight PointLights[MAX_POINT_LIGHT_NUM];
	SpotLight SpotLights[MAX_SPOT_LIGHT_NUM];
};

//----Sprite----//
struct CB_Sprite
{
	CB_Sprite()
	{
		SpriteColor = Color(1, 1, 1, 1);
		ScreenSize = Vector2(0, 0);
		Pos = Vector2(0, 0);
		Scale = Vector2(1, 1);
		TexSize = Vector2(1, 1);
		TexSamplePos = Vector2(0, 0);
		TexSampleSize = Vector2(0, 0);
		Z = 0;
		Alpha = 1.f;
		PosOffset = Vector2(0, 0);
		//pad0 = 0;
		//pad1 = 0;
	}
	Color SpriteColor;
	Vector2 ScreenSize;
	Vector2 Pos;
	Vector2 Scale;
	Vector2 TexSize;
	Vector2 TexSamplePos;
	Vector2 TexSampleSize;
	float Z;
	float Alpha;
	Vector2 PosOffset;
	//float pad0;
	//float pad1;
};

//----Particle----//
struct CB_ParticleSystem
{
	float CustomData1[4];
	float CustomData2[4];
	float Time = 0.f;
	float CurrentTime = 0.f;
	Vector2 pad0;
	Color StartColor;

	float DeltaTime;
	float MaxParticles;
	Vector2 AnimationUV_Offset;
	Vector2 AnimationUV_Scale;

	Vector2 ParticleSize = Vector2(1,1);
	Vector2 pad1;
};

struct ComputeParticle
{
	Vector4 Position;
	Vector3 Direction;
	float Speed;
	float RotationAngle;
	float DelayTime = 0.f;
};