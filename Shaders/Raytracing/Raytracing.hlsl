#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingHelper.hlsli"

#define SHADOW_ON

#define MAX_POINT_LIGHT_NUM 100
#define MAX_SPOT_LIGHT_NUM 100

#define HitDistanceOnMiss 0

//-----------GLOBAL-----------//
RWTexture2D<float4> g_renderTarget : register(u0);
RWTexture2D<float4> g_rtGBufferPosition : register(u1);
RWTexture2D<float> g_rtGBufferDepth : register(u2);

RaytracingAccelerationStructure g_scene : register(t0, space0);
TextureCube<float4> g_texEnvironmentMap : register(t1);
SamplerState LinearWrapSampler : register(s0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<LightList> g_lightList : register(b1);

//-----------LOCAL-----------//
StructuredBuffer<uint> l_indices : register(t0, space1);
StructuredBuffer<PositionNormalUVTangentColor> l_vertices : register(t1, space1);
Texture2D<float4> l_texDiffuse : register(t2, space1);
Texture2D<float4> l_texNormal : register(t3, space1);
Texture2D<float4> l_texMask : register(t4, space1);
ConstantBuffer<MaterialInfoConstantBuffer> l_materialInfo : register(b0, space1);
//StructuredBuffer<MaterialInfoConstantBuffer> l_materialInfo : register(b0, space1);


typedef BuiltInTriangleIntersectionAttributes MyAttributes;

// Light

struct GBuffer
{
    float tHit;
    float3 hitPosition;
};

struct RayPayload
{
    unsigned int rayRecursionDepth;
    float3 radiance;
    GBuffer gBuffer;
};

struct ShadowRayPayload
{
    float tHit;
};

inline float3 GenerateForwardCameraRayDirection(in float4x4 projectionToWorld)
{
    float2 screenPos = float2(0, 0);
	
	// Unproject the pixel coordinate into a world positon.
	float4 world = mul(float4(screenPos, 0, 1), projectionToWorld);
	return normalize(world.xyz);
}

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);

}

float2 HitAttribute(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 NormalMap(in float3 normal, in float2 texCoord, in PositionNormalUVTangentColor vertices[3], in MyAttributes attr, in float lod)
{
    float3 tangent;
    float3 vertexTangents[3] =
    {
        vertices[0].tangent,
        vertices[1].tangent,
        vertices[2].tangent
    };
    tangent = HitAttribute(vertexTangents, attr);
    {
        ////
        float3 v0 = vertices[0].position;
        float3 v1 = vertices[1].position;
        float3 v2 = vertices[2].position;
        float2 uv0 = float2(vertices[0].uv[0], vertices[0].uv[1]);
        float2 uv1 = float2(vertices[1].uv[0], vertices[1].uv[1]);
        float2 uv2 = float2(vertices[2].uv[0], vertices[2].uv[1]);
        tangent = CalculateTangent(v0, v1, v2, uv0, uv1, uv2);
    }

    float3 texSample = l_texNormal.SampleLevel(LinearWrapSampler, texCoord, lod).xyz;
    float3 newNormal;
    float3 bumpNormal = normalize(texSample * 2.f - 1.f);
    float3 worldNormal = BumpMapNormalToWorldSpaceNormal(bumpNormal, normal, tangent);
    return worldNormal;
}

RayPayload TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth, float tMin = 0.001f, float tMax = 10000.f, bool cullNonOpaque = false)
{
    RayPayload payload;
    payload.rayRecursionDepth = currentRayRecursionDepth + 1;
    payload.radiance = float3(0, 0, 0);
    payload.gBuffer.tHit = HitDistanceOnMiss;
    payload.gBuffer.hitPosition = 0;
    
    if (currentRayRecursionDepth >= g_sceneCB.maxRadianceRayRecursionDepth)
    {
        payload.radiance = float3(133, 161, 179) / 255.0;
        //payload.radiance = float3(0, 0, 0) / 255.0;
        return payload;
    }
    
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = tMin;
    rayDesc.TMax = tMax;
    //rayDesc.TMin = g_sceneCB.nearZ;
    //rayDesc.TMax = g_sceneCB.farZ;
    
    UINT rayFlags = (cullNonOpaque ? RAY_FLAG_CULL_NON_OPAQUE : 0);
    //UINT rayFlags;// = RAY_FLAG_CULL_NON_OPAQUE;

    //rayFlags |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    
    //UINT rayFlags = 0;
    //UINT rayFlags = RAY_FLAG_CULL_NON_OPAQUE;
    TraceRay(
        g_scene,
        rayFlags,
        ~0, // instance mask
        0, // offset - path tracer radiance
        2, // Pathtacer ray type count
        0, // offset - path tracer radiance
        rayDesc,
        payload);
    return payload;
}

float3 TraceReflectedRay(in float3 hitPosition, in float3 wi, inout RayPayload rayPayload, in float TMax = 3000)
{

    float tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wi; // 자기 교차 방지!
    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray;
    ray.origin = adjustedHitPosition;
    ray.direction = wi;
    float tMin = 0;
    float tMax = TMax;

    rayPayload = TraceRadianceRay(ray, rayPayload.rayRecursionDepth, tMin, tMax);
    if(rayPayload.gBuffer.tHit != HitDistanceOnMiss)
    {
        rayPayload.gBuffer.tHit += RayTCurrent() + tOffset;
    }

    return rayPayload.radiance;
}

float3 TraceRefractedRay(in float3 hitPosition, in float3 wt, in float3 N, in float3 objectNormal, inout RayPayload rayPayload, in float TMax = 10000)
{
    float tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wt;
    float3 adjustedHitposition = hitPosition + offsetAlongRay;
    Ray ray = {adjustedHitposition, wt};
    float tMin = 0;
    float tMax = TMax;
   
    bool cullNonOpaque = true;
    int recursionDepth = rayPayload.rayRecursionDepth;
    // recursionDepth = clamp(recursionDepth - 1, 0, 2);  // 0보단 작아지지 않게
    //rayPayload = TraceRadianceRay(ray, rayPayload.rayRecursionDepth, tMin, tMax, cullNonOpaque);
    rayPayload = TraceRadianceRay(ray, recursionDepth, tMin, tMax, cullNonOpaque);

    if(rayPayload.gBuffer.tHit != HitDistanceOnMiss)
    {
        rayPayload.gBuffer.tHit += RayTCurrent() + tOffset;
    }

    return rayPayload.radiance;
}

// 그림자 레이를 쏘고 geometry에 맞으면 true 반환
bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in UINT currentRayRecursionDepth, in bool retrieveTHit = true, in float TMax = 10000)
{
    tHit = 0;
    if (currentRayRecursionDepth >= g_sceneCB.maxShadowRayRecursionDepth)
    {
        return true;
    }
    
    // 확장된 레이 세팅
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.0;
    rayDesc.TMax = TMax;
    
    ShadowRayPayload shadowPayload = { TMax };
    
    UINT rayFlags = 0; 
    rayFlags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    rayFlags |= RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
 
    
    TraceRay(
        g_scene,
        rayFlags,
        ~0,
        1, // ray type shadow
        2, // geometry stride - path tracer count
        1, // ray type shadow
        rayDesc,
        shadowPayload);
    tHit = shadowPayload.tHit;
    
    return shadowPayload.tHit > 0;
}

bool TraceShadowRayAndReportIfHit(out float tHit, in Ray ray, in float3 N, in UINT currentRayRecursionDepth, in bool retrieveTHit = true, in float TMax = 10000)
{
    tHit = 0;
    if (dot(ray.direction, N) > 0)
    {
        return TraceShadowRayAndReportIfHit(tHit, ray, currentRayRecursionDepth, retrieveTHit, TMax);
    }
    return false;
}

bool TraceShadowRayAndReportIfHit(in float3 hitPosition, in float3 direction, in float3 N, in RayPayload rayPayload, in float TMax = 10000)
{
    float tOffset = 0.001f;
    Ray visibilityRay = { hitPosition + tOffset * N, direction };
    float dummyTHit = 0;
    return TraceShadowRayAndReportIfHit(dummyTHit, visibilityRay, N, rayPayload.rayRecursionDepth, false, TMax);
}

void CalculateSpecular(
    in float3 Albedo,
    in float metallic,
    in float roughness,
    in float3 V,
    in float3 N,
    out float3 Ks,
    out float3 Kr)
{
   
  // F0 계산
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), Albedo.rgb, metallic);
    
    // Fresnel-Schlick 근사 반사율 계산
    float dotNV = saturate(dot(N, V));
    float3 reflectance = F0 + (1.0 - F0) * pow(1.0 - dotNV, 5.0);
    
    // Smoothness를 기반으로 반사율 조정
    reflectance *= 1 - roughness;//smoothness;
    
    // 최종 반사율 사용
    Ks = reflectance;
    Kr = reflectance;
    return;
}

float3 Shade(
    inout RayPayload rayPayload,
    float2 uv,
    in float3 N,
    in float3 objectNormal,
    in float3 hitPosition,
    in float lod
)
{
    float3 V = -WorldRayDirection();
    float3 L = 0;

    float distance = length(g_sceneCB.cameraPosition.xyz - hitPosition);

    // 거리와 법선 각도를 기반으로 LOD 값 계산
    float4 baseTex = l_texDiffuse.SampleLevel(LinearWrapSampler, uv, lod);
    float3 albedo = baseTex.xyz;

    float3 Kd = baseTex.xyz;
    float3 Ks;
    float3 Kr;
    const float3 Kt = float3(1,1,1);
    float metallic;
    float roughness;

    float4 maskSample = l_texMask.SampleLevel(LinearWrapSampler, uv, lod);

    metallic = maskSample.x;
    roughness = 1 - maskSample.a;
    float ao = maskSample.g;
    
    CalculateSpecular(Kd, metallic, roughness, V, N, Ks, Kr);
    

    if (!BxDF::IsBlack(Kd) || !BxDF::IsBlack(Ks))
    {
        int dirLightNum = g_lightList.DirLightNum;
        int pointLightNum = g_lightList.PointLightNum;
        int spotLightNum = g_lightList.SpotLightNum;
    
        // Directional Light
        {
            for(int i = 0; i < dirLightNum; ++i)
            {
                float3 LightVector = -g_lightList.DirLights[i].Direction.xyz;
                float3 H = normalize(V + LightVector);
                float3 radiance = g_lightList.DirLights[i].DiffuseColor.rgb;
                float intensity = g_lightList.DirLights[i].Intensity;
                bool isInShadow = TraceShadowRayAndReportIfHit(hitPosition, LightVector, N, rayPayload);
                L = DirectionalLight(isInShadow, V, LightVector, N, radiance, albedo, roughness, metallic, intensity);
            }
        }
        
        {   
            // Point Light
            for (int i = 0; i < pointLightNum; ++i)
            {
                float3 position = g_lightList.PointLights[i].Position.xyz;
                float3 color = g_lightList.PointLights[i].Color.rgb;
                float range = g_lightList.PointLights[i].Range;
                float intensity = g_lightList.PointLights[i].Intensity;
                float3 direction = normalize(position - hitPosition);
                float distance = length(position - hitPosition);
                bool isInShadow = false;

                uint lightCast = l_materialInfo.Layer & g_lightList.PointLights[i].Layer;
                lightCast = clamp(0, 1, lightCast);
                range *= (float)lightCast;

                if(distance <= range)
                {
                    isInShadow = TraceShadowRayAndReportIfHit(hitPosition, direction, N, rayPayload, distance);
                    //isInShadow *= g_lightList.PointLights[i].IsNoShadowCasting;
                    uint shadowCast = (uint)isInShadow;
                    shadowCast *= g_lightList.PointLights[i].IsShadowCasting;
                    L += PointLight((bool)shadowCast, V, direction, N, distance, color, albedo, roughness, metallic, intensity);
                }
            }
            // SpotLight
            for (int i = 0; i < spotLightNum; ++i)
            {
                float3 position = g_lightList.SpotLights[i].Position.xyz;
                float3 color = g_lightList.SpotLights[i].Color.rgb;
                float range = g_lightList.SpotLights[i].Range;
                float angle = g_lightList.SpotLights[i].SpotAngle;
                float intensity = g_lightList.SpotLights[i].Intensity;
                float softness = g_lightList.SpotLights[i].Softness;
                float3 direction = normalize(position - hitPosition);
                float distance = length(position - hitPosition);
                float3 lightDirection = normalize(g_lightList.SpotLights[i].Direction.xyz);
                
                bool isInShadow = false;
                if (distance <= range)
                {
                    isInShadow = TraceShadowRayAndReportIfHit(hitPosition, direction, N, rayPayload, distance);
                    L += SpotLight(isInShadow, V, direction, lightDirection, N, distance, color, softness, angle, albedo, roughness, metallic, intensity, range);
                }
            }
        }
    }

    L += g_sceneCB.AmbientIntensity * albedo;// * ao;
    L *= ao;

    bool isReflective = !BxDF::IsBlack(Kr);
    bool isTransmissive = l_materialInfo.bIsTransmissive;
    
    float smallValue = 1e-6f;
    isReflective = dot(V, N) > smallValue ? isReflective : false;
    if (isReflective || isTransmissive)
    {
        float range = 3000.f * pow(maskSample.a * 0.9f + 0.1f, 4.0f);
        //if (isReflective && (BxDF::Specular::Reflection::IsTotalInternalReflection(V, N)))
        //{
        //    RayPayload reflectedPayLoad = rayPayload;
        //    float3 wi = reflect(-V, N);
        //    
        //    L += Kr * TraceReflectedRay(hitPosition, wi, reflectedPayLoad, range);
        //}
        //else // No total internal reflection
        {
            float3 Fo = Ks;
            if (isReflective)
            {
                // 반사 벡터
                float3 wi;
                // 뷰 방향 벡터로부터 스펙큘러 반사를 위한 방향 벡터를 계산하고, 이를 기반으로 프레넬 반사도를 반환
                float3 Fr = Kr * BxDF::Specular::Reflection::Sample_Fr(V, wi, N, Fo); // Calculates wi
                RayPayload reflectedRayPayLoad = rayPayload;
                L += Fr * TraceReflectedRay(hitPosition, wi, reflectedRayPayLoad, range);
                
            }
            if(isTransmissive)
            {
                if(baseTex.a < 0.5f)
                {
                    float3 wt;
                    float3 Ft = Kt * BxDF::Specular::Transmission::Sample_Ft(V, wt, N, Fo);
                    RayPayload refractedRayPayLoad = rayPayload;
                    L += Ft * TraceRefractedRay(hitPosition, wt, N, objectNormal, refractedRayPayLoad);
                }
            }
        }
    }
    return L;
}

inline Ray GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // 픽셀의 중간으로 이동
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // DirectX 좌표계를 위해 Y를 반전시킨다.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    // 투영행렬의 역행렬을 곱하여 Ray가 향하게 될 좌표값에 따른 Near Plane의 좌표를 구한다.
    float4 world = mul(float4(screenPos, 0, 1), g_sceneCB.projectionToWorld);
    world.xyz /= world.w;
    
    origin = g_sceneCB.cameraPosition.xyz;
    // 최종 방향 결정
    direction = normalize(world.xyz - origin);

    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    
    return ray;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    
    uint2 dispatchRayIndex = DispatchRaysIndex().xy;
    // 카메라에서 화면으로 향하는 Ray를 생성한다.
    Ray ray = GenerateCameraRay(dispatchRayIndex, origin, rayDir);
    
    UINT currentRayRecursionDepth = 0;
    RayPayload rayPayload = TraceRadianceRay(ray, currentRayRecursionDepth, 0.001f, 3000.f);

    g_renderTarget[DispatchRaysIndex().xy] = float4(rayPayload.radiance, 1);

    //--- GBuffer ---//
    
    g_rtGBufferPosition[dispatchRayIndex] = float4(rayPayload.gBuffer.hitPosition, 1);
    
    bool hasCameraRayHitGeometry = rayPayload.gBuffer.tHit != HitDistanceOnMiss;
    float rayLength = HitDistanceOnMiss;
    if(hasCameraRayHitGeometry)
    {
        float4 clipSpacePos = mul(mul(float4(rayPayload.gBuffer.hitPosition, 1.0f), g_sceneCB.View), g_sceneCB.Proj);
        float3 ndcPos = clipSpacePos.xyz / clipSpacePos.w;
        //float depth = (ndcPos.z * 0.5) + 0.5;
        //float depth = (ndcPos.z + 1.f) * 0.5;
        float depth = ndcPos.z;
        g_rtGBufferDepth[dispatchRayIndex] = depth;
    }
    else
    {
        g_rtGBufferDepth[dispatchRayIndex] = 1;
    }
    return;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;

    const uint3 indices = uint3(
         l_indices[baseIndex],
         l_indices[baseIndex + 1],
         l_indices[baseIndex + 2]
     );

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3] =
    {
        l_vertices[indices[0]].normal,
        l_vertices[indices[1]].normal,
        l_vertices[indices[2]].normal 
    };
    
    float3 vertexPositions[3] =
    {
        l_vertices[indices[0]].position,
        l_vertices[indices[1]].position,
        l_vertices[indices[2]].position 
    };
    
    float3 vertexTangents[3] =
    {
        l_vertices[indices[0]].tangent,
        l_vertices[indices[1]].tangent,
        l_vertices[indices[2]].tangent
    };

    PositionNormalUVTangentColor vertexInfo[3] =
    {
        l_vertices[indices[0]],
        l_vertices[indices[1]],
        l_vertices[indices[2]]
    };

    float2 vertexTexCoords[3] = { vertexInfo[0].uv, vertexInfo[1].uv, vertexInfo[2].uv };
    float2 uv = HitAttribute(vertexTexCoords, attr);
    
    float2 tiling = float2(l_materialInfo.TilingX, l_materialInfo.TilingY);
    float2 offset = float2(l_materialInfo.OffsetX, l_materialInfo.OffsetY);
    Ideal_TilingAndOffset_float(uv, tiling, offset, uv);

    // Normal
    float3 normal;  // 최종 노말
    float3 objectNormal; // 삼각형이 향하는 노말
    {
        float3 vertexNormals[3] =
        {
            vertexInfo[0].normal,
            vertexInfo[1].normal,
            vertexInfo[2].normal
        };
        objectNormal = normalize(HitAttribute(vertexNormals, attr));
        float orientation = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? 1 : -1;
        objectNormal *= orientation;

        normal = normalize(mul((float3x3)ObjectToWorld3x4(), objectNormal));
    }
    
    float lod;
    {
        float distance = length(g_sceneCB.cameraPosition.xyz - hitPosition);
        ///// calculate lod
        // calculate alpha
        float alpha = 2 * atan(g_sceneCB.FOV / g_sceneCB.resolutionY * 2);
        float coneWidth = 2 * distance * tan(alpha / 2);

        // calculate texLOD
        float2 vTriUVInfo = TriUVInfoFromRayCone(
        WorldRayDirection(), normal, coneWidth,
        vertexTexCoords, vertexPositions, (float3x3) ObjectToWorld3x4()
        );
        
        uint2 texSize;
        l_texDiffuse.GetDimensions(texSize.x, texSize.y);
        float MipIndex = TriUVInfoToTexLOD(texSize, vTriUVInfo);
        lod = MipIndex;
        lod *= 0.7f;
    }
    {
        normal = NormalMap(normal, uv, vertexInfo, attr, lod);
    }
    
    // GBuffer
    payload.gBuffer.tHit = RayTCurrent();
    payload.gBuffer.hitPosition = hitPosition;

    payload.radiance = Shade(payload, uv, normal, objectNormal, hitPosition, lod);
}

[shader("closesthit")]
void MyClosestHitShader_ShadowRay(inout ShadowRayPayload rayPayload, in MyAttributes attr)
{
    rayPayload.tHit = RayTCurrent();
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float3 radiance = g_texEnvironmentMap.SampleLevel(LinearWrapSampler, WorldRayDirection(), 0).xyz;
    payload.radiance = radiance;
}

[shader("miss")]
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload)
{
    rayPayload.tHit = HitDistanceOnMiss;
}

[shader("anyhit")]
void MyAnyHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    const uint3 indices = uint3(
         l_indices[baseIndex],
         l_indices[baseIndex + 1],
         l_indices[baseIndex + 2]
     );
    PositionNormalUVTangentColor vertexInfo[3] =
    {
        l_vertices[indices[0]],
        l_vertices[indices[1]],
        l_vertices[indices[2]]
    };
    float2 vertexTexCoords[3] = { vertexInfo[0].uv, vertexInfo[1].uv, vertexInfo[2].uv };
    float2 uv = HitAttribute(vertexTexCoords, attr);
    float alpha = l_texDiffuse.SampleLevel(LinearWrapSampler, uv, 0).a;
    if(alpha < 0.5f && l_materialInfo.bIsTransmissive == false)    // threshold
    {
        IgnoreHit();
    }
}

[shader("anyhit")]
void MyAnyHitShader_ShadowRay(inout ShadowRayPayload payload, in MyAttributes attr)
{

    if(l_materialInfo.bIsTransmissive)
    {
        IgnoreHit();
        return;
    }

    payload.tHit = RayTCurrent();
    float3 hitPosition = HitWorldPosition();
    uint baseIndex = PrimitiveIndex() * 3;
    const uint3 indices = uint3(
         l_indices[baseIndex],
         l_indices[baseIndex + 1],
         l_indices[baseIndex + 2]
     );
    PositionNormalUVTangentColor vertexInfo[3] =
    {
        l_vertices[indices[0]],
        l_vertices[indices[1]],
        l_vertices[indices[2]]
    };
    float2 vertexTexCoords[3] = { vertexInfo[0].uv, vertexInfo[1].uv, vertexInfo[2].uv };
    float2 uv = HitAttribute(vertexTexCoords, attr);
    float alpha = l_texDiffuse.SampleLevel(LinearWrapSampler, uv, 0).a;

    if(alpha < 0.5f)    // threshold
    {
        IgnoreHit();
        return;
    }
}
#endif // RAYTRACING_HLSL