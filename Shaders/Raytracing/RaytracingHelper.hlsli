#ifndef RAYTRACINGSHADERHELPER_H
#define RAYTRACINGSHADERHELPER_H


#define PI 3.141592

namespace BxDF
{
    bool IsBlack(float3 color)
    {
        return !any(color);
    }
    namespace Diffuse
    {
        namespace Lambert
        {
            float3 F(in float3 albedo)
            {
                return albedo;
            }
        }
        
        float3 F(in float3 Albedo, in float Roughness, in float3 N, in float3 V, in float3 L, in float3 Fo)
        {
            float3 diffuse = 0;

            float3 H = normalize(V + L);
            float NoH = dot(N, H);
            if (NoH > 0)
            {
                float a = Roughness * Roughness;

                float NoV = saturate(dot(N, V));
                float NoL = saturate(dot(N, L));
                float LoV = saturate(dot(L, V));

                float facing = 0.5 + 0.5 * LoV;
                float rough = facing * (0.9 - 0.4 * facing) * ((0.5 + NoH) / NoH);
                float3 smooth = 1.05f * (1 - pow(1 - NoL, 5)) * (1 - pow(1 - NoV, 5));

                    // Extract 1 / PI from the single equation since it's ommited in the reflectance function.
                float3 single = lerp(smooth, rough, a);
                float multi = 0.3641 * a; // 0.3641 = PI * 0.1159

                diffuse = Albedo * (single + Albedo * multi);
                return diffuse;
            }
            return Albedo * 0.5;
        }
    }
    // Fresnel reflectance - schlick approximation.
    float3 Fresnel(in float3 F0, in float cos_thetai)
    {
        return F0 + (1 - F0) * pow(1 - cos_thetai, 5);
    }
    
    namespace Specular
    {
        namespace Reflection
        {
            // Calculates L and returns BRDF value for that direction.
            // Assumptions: V and N are in the same hemisphere.
            // Note: to avoid unnecessary precision issues and for the sake of performance the function doesn't divide by the cos term
            // so as to nullify the cos term in the rendering equation. Therefore the caller should skip the cos term in the rendering equation.
            float3 Sample_Fr(in float3 V, out float3 L, in float3 N, in float3 Fo)
            {
                L = reflect(-V, N);
                float cos_thetai = dot(N, L);
                return Fresnel(Fo, cos_thetai);
            }
            
            // Calculate whether a total reflection occurs at a given V and a normal
            // Ref: eq 27.5, Ray Tracing from the Ground Up
            bool IsTotalInternalReflection(
                in float3 V,
                in float3 normal)
            {
                float ior = 1;
                float eta = ior;
                float cos_thetai = dot(normal, V); // Incident angle

                return 1 - 1 * (1 - cos_thetai * cos_thetai) / (eta * eta) < 0;
            }
        }
        
        namespace Transmission
        {
            // Calculates transmitted ray wt and return BRDF value for that direction.
            // Assumptions: V and N are in the same hemisphere.
            // Note: to avoid unnecessary precision issues and for the sake of performance the function doesn't divide by the cos term
            // so as to nullify the cos term in the rendering equation. Therefore the caller should skip the cos term in the rendering equation.
            float3 Sample_Ft(in float3 V, out float3 wt, in float3 N, in float3 Fo)
            {
                float ior = 1;
                wt = -V; // TODO: refract(-V, N, ior);
                float cos_thetai = dot(V, N);
                float3 Kr = Fresnel(Fo, cos_thetai);

                return (1 - Kr);
            }
        }
        
        namespace GGX
        {
             // Compute the value of BRDF
            float3 F(in float Roughness, in float3 N, in float3 V, in float3 L, in float3 Fo)
            {
                float3 H = V + L;
                float NoL = dot(N, L);
                float NoV = dot(N, V);
                float3 specular = 0;

                if (NoL > 0 && NoV > 0 && all(H))
                {
                    H = normalize(H);
                    float a = Roughness; // The roughness has already been remapped to alpha.
                    float3 M = H; // microfacet normal, equals h, since BRDF is 0 for all m =/= h. Ref: 9.34, RTR
                    float NoM = saturate(dot(N, M));
                    float HoL = saturate(dot(H, L));

                    // D
                    // Ref: eq 9.41, RTR
                    float denom = 1 + NoM * NoM * (a * a - 1);
                    float D = a * a / (denom * denom); // Karis

                    // F
                    // Fresnel reflectance - Schlick approximation for F(h,l)
                    // Ref: 9.16, RTR
                    float3 F = Fresnel(Fo, HoL);

                    // G
                    // Visibility due to shadowing/masking of a microfacet.
                    // G coupled with BRDF's {1 / 4 DotNL * DotNV} factor.
                    // Ref: eq 9.45, RTR
                    float G = 0.5 / lerp(2 * NoL * NoV, NoL + NoV, a);

                    // Specular BRDF term
                    // Ref: eq 9.34, RTR
                    specular = F * G * D;
                }

                return specular;
            }
        }
    }
    
    namespace DirectLighting
    {
        float3 Shade(
        in float3 Albedo,
        in float3 Fo,
        in float3 Radiance,
        in bool isInShadow,
        in float Roughness,
        in float3 N,
        in float3 V,
        in float3 L)
        {
            float3 directLighting = 0;
            
            float NoL = dot(N, L);
            if (!isInShadow && NoL > 0)
            {
                float3 directDiffuse = 0;
                if (!IsBlack(Albedo))
                {
                    directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    //if(directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    //{
                    //    directDiffuse = Albedo;
                    //}
                    //if (IsBlack(directDiffuse))
                    //{
                    //    directDiffuse = Albedo;
                    //}
                    //if (directDiffuse.x > float3(1.f, 1.f, 1.f))
                    //{
                    //    directDiffuse = Albedo.xyz;
                    //}

                }
                
                float3 directSpecular = 0;
                directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);
                directLighting = NoL * Radiance * (directDiffuse + directSpecular);
                //if(IsBlack(directLighting))
                //{
                //    directLighting = Albedo;
                //}
            }
            return directLighting;
        }
    }
}

// Sample normal map, convert to signed, apply tangent-to-world space transform.
float3 BumpMapNormalToWorldSpaceNormal(float3 bumpNormal, float3 surfaceNormal, float3 tangent)
{
    // Compute tangent frame.
    surfaceNormal = normalize(surfaceNormal);
    tangent = normalize(tangent);

    float3 bitangent = normalize(cross(tangent, surfaceNormal));
    float3x3 tangentSpaceToWorldSpace = float3x3(tangent, bitangent, surfaceNormal);

    return normalize(mul(bumpNormal, tangentSpaceToWorldSpace));
}


namespace Ideal
{
    bool CheckReflect(float3 Kr)
    {
        if (Kr.r < 0.05f)
            return false;
        return true;
    }
    float Attenuate(float distance, float range)
    {
        // float att = saturate(1.f - (distance * distance / (range * range)));
       // return att * att;
        
        float numer = distance / range;
        numer = numer * numer;
        numer = numer * numer;
        numer = saturate(1 - numer);
        numer = numer * numer;
        float denom = distance * distance + 1;
        return (numer / denom);
    }
    
    namespace Light
    {
        float3 ComputeDirectionalLight(
        in float3 Albedo,
        in float3 Fo,
        in float3 Radiance,
        in bool isInShadow,
        in float Roughness,
        in float3 N,
        in float3 V,
        in float3 DirectionalLightVector
        )
        {
            float3 directLighting = 0;
            float3 L = -DirectionalLightVector;
            
            //float NoL = dot(N, L);
            float NoL = dot(L, N);
            if (!isInShadow && NoL > 0)
            {
                float3 directDiffuse = 0;
                if (!BxDF::IsBlack(Albedo))
                {
                    directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    if (directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    {
                        directDiffuse = Albedo;
                    }
                    //directDiffuse = BxDF::Diffuse::Lambert::F(Albedo);
                    if(directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    //if (BxDF::IsBlack(directDiffuse))
                    {
                        directDiffuse = Albedo;
                    }
                }
                
                float3 directSpecular = 0;
                directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);
                directLighting = NoL * Radiance * (directDiffuse + directSpecular);
            }
            return directLighting;
        }
        
        float3 ComputePointLight(
        in float3 Albedo,
        in float3 Fo,
        in float3 Radiance,
        in bool isInShadow,
        in float Roughness,
        in float3 N,
        in float3 V,
        in float3 L,
        in float LightDistance,
        in float LightRange,
        in float LightIntensity
        )
        {
            float3 directLighting = 0;
            
            if (LightDistance > LightRange)
            {
                return directLighting;
            }
            
            float NoL = dot(N, L);
            if (!isInShadow && NoL > 0)
            {
                 // 임의 추가
                float attenuation = Attenuate(LightDistance, LightRange);
                float intensity = LightIntensity * attenuation;
                    
                float3 directDiffuse = 0;
                float3 directSpecular = 0;
                
                if (!BxDF::IsBlack(Albedo))
                {
                    //directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    if (directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    {
                        directDiffuse = Albedo;
                    }
                    //directDiffuse = BxDF::Diffuse::Lambert::F(Albedo);
                    if (directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    //if (BxDF::IsBlack(directDiffuse))
                    {
                        directDiffuse = Albedo;
                    }
                }
                
                directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);
                
                directDiffuse *= intensity;
                directSpecular *= intensity;
                
                directLighting = NoL * Radiance * (directDiffuse + directSpecular);
            }
            return directLighting;
        }
        
        float3 ComputeSpotLight(
        in float3 Albedo,
        in float3 Fo,
        in float3 Radiance,
        in bool isInShadow,
        in float Roughness,
        in float3 N,
        in float3 V,
        in float3 L,
        in float LightDistance,
        in float LightRange,
        in float LightIntensity,
        in float3 LightDirection,
        in float SpotAngle,
        in float SpotPenumbra
        )
        {
            float3 directLighting = 0;

            if (LightDistance > LightRange)
            {
                return directLighting;
            }
            float NoL = dot(N, L);
            float NoV = dot(N, V);
            if (!isInShadow && NoL > 0 && NoV > 0)
            {
        // 조명 감쇠 계산
                float attenuation = Attenuate(LightDistance, LightRange);

        // 스포트라이트 각도 계산
                float cosTheta = dot(normalize(-L), normalize(LightDirection));
                float spotEffect = smoothstep(cos(SpotAngle), cos(SpotAngle + SpotPenumbra), cosTheta);
                //float innerCone = cos(radians(SpotAngle));
                //float outerCone = cos(radians(SpotAngle + SpotPenumbra));
                //float spotEffect = smoothstep(outerCone, innerCone, cosTheta);

                float intensity = LightIntensity * attenuation * spotEffect;

                float3 directDiffuse = 0;
                float3 directSpecular = 0;

                if (!BxDF::IsBlack(Albedo))
                {
                    //directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    if (directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    {
                        directDiffuse = Albedo;
                    }
                    //directDiffuse = BxDF::Diffuse::Lambert::F(Albedo);
                    if (directDiffuse.x > 1 && directDiffuse.y > 1 && directDiffuse.z > 1)
                    //if (BxDF::IsBlack(directDiffuse))
                    {
                        directDiffuse = Albedo;
                    }
                }

                directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);

        // 조명 강도 적용
                directDiffuse *= intensity;
                directSpecular *= intensity;

                directLighting = NoL * Radiance * (directDiffuse + directSpecular);
            }
            return directLighting;
        }
        float3 ComputeSpotLight2(
    in float3 Albedo,
    in float3 Fo,
    in float3 Radiance,
    in bool isInShadow,
    in float Roughness,
    in float3 N,
    in float3 V,
    in float3 L,
    in float LightDistance,
    in float LightRange,
    in float LightIntensity,
    in float LightSoftness,
    in float3 SpotDirection,
    in float SpotAngle
)
        {
            float3 directLighting = 0;

            if (LightDistance > LightRange)
            {
                return directLighting;
            }

            float NoL = dot(N, L);
            if (!isInShadow && NoL > 0)
            {
        // 스포트라이트의 각도 계산
                float spotEffect = dot(normalize(-L), SpotDirection);
                float cutoff = cos(radians(SpotAngle * 0.5));
                
                //float SpotSoftness = 10.0f;
                //float SpotSoftness = 1.0f;
                float SpotSoftness = LightSoftness;
                float outerCutoff = cos(radians((SpotAngle * 0.5) + SpotSoftness));
        
                    // 스포트라이트 감쇠 적용 (부드러운 가장자리 효과)
                float smoothFactor = smoothstep(outerCutoff, cutoff, spotEffect);
                
                if (spotEffect > outerCutoff)
                {
            // 스포트라이트의 감쇠 계산
                    float attenuation = Attenuate(LightDistance, LightRange);
                    float intensity = LightIntensity * attenuation * smoothFactor; // 스포트라이트 효과 강화

                    float3 directDiffuse = 0;
                    float3 directSpecular = 0;

                    if (!BxDF::IsBlack(Albedo))
                    {
                        directDiffuse = BxDF::Diffuse::F(Albedo, Roughness, N, V, L, Fo);
                    }

                    directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);

                    directDiffuse *= intensity;
                    directSpecular *= intensity;

                    directLighting = NoL * Radiance * (directDiffuse + directSpecular);
                }
            }
            return directLighting;
        }
    }
}

namespace nvidia
{
    //https://developer.nvidia.com/blog/solving-self-intersection-artifacts-in-directx-raytracing/
    // Compute the object and world space position and normal corresponding to a triangle hit point.
    // Compute a safe spawn point offset along the normal in world space to prevent self intersection of secondary rays.
    void safeSpawnPoint(
    out float3 outObjPosition, // position in object space
    out float3 outWldPosition, // position in world space    
    out float3 outObjNormal, // unit length surface normal in object space
    out float3 outWldNormal, // unit length surface normal in world space
    out float outWldOffset, // safe offset for spawn position in world space
    const float3 v0, // spawn triangle vertex 0 in object space
    const float3 v1, // spawn triangle vertex 1 in object space
    const float3 v2, // spawn triangle vertex 2 in object space
    const float2 bary, // spawn barycentrics
    const float3x4 o2w, // spawn instance object-to-world transformation
    const float3x4 w2o)           // spawn instance world-to-object transformation
    {
        precise float3 edge1 = v1 - v0;
        precise float3 edge2 = v2 - v0;

    // interpolate triangle using barycentrics.
    // add in base vertex last to reduce object space error.
        precise float3 objPosition = v0 + mad(bary.x, edge1, mul(bary.y, edge2));
        float3 objNormal = cross(edge1, edge2);

    // transform object space position.
    // add in translation last to reduce world space error.
        precise float3 wldPosition;
        wldPosition.x = o2w._m03 +
        mad(o2w._m00, objPosition.x,
            mad(o2w._m01, objPosition.y,
                mul(o2w._m02, objPosition.z)));
        wldPosition.y = o2w._m13 +
        mad(o2w._m10, objPosition.x,
            mad(o2w._m11, objPosition.y,
                mul(o2w._m12, objPosition.z)));
        wldPosition.z = o2w._m23 +
        mad(o2w._m20, objPosition.x,
            mad(o2w._m21, objPosition.y,
                mul(o2w._m22, objPosition.z)));

    // transform normal to world - space using
    // inverse transpose matrix
        float3 wldNormal = mul(transpose((float3x3) w2o), objNormal);

    // normalize world space normal
        const float wldScale = rsqrt(dot(wldNormal, wldNormal));
        wldNormal = mul(wldScale, wldNormal);

        const float c0 = 5.9604644775390625E-8f;
        const float c1 = 1.788139769587360206060111522674560546875E-7f;

        const float3 extent3 = abs(edge1) + abs(edge2) + abs(edge1 - edge2);
        const float extent = max(max(extent3.x, extent3.y), extent3.z);

    // bound object space error due to reconstruction and intersection
        float3 objErr = mad(c0, abs(v0), mul(c1, extent));

    // bound world space error due to object to world transform
        const float c2 = 1.19209317972490680404007434844970703125E-7f;
        float3 wldErr = mad(c1, mul(abs((float3x3) o2w), abs(objPosition)), mul(c2, abs(transpose(o2w)[3])));

    // bound object space error due to world to object transform
        objErr = mad(c2, mul(abs(w2o), float4(abs(wldPosition), 1)), objErr);

    // compute world space self intersection avoidance offset
        float wldOffset = dot(wldErr, abs(wldNormal));
        float objOffset = dot(objErr, abs(objNormal));

        wldOffset = mad(wldScale, objOffset, wldOffset);

    // output safe front and back spawn points
        outObjPosition = objPosition;
        outWldPosition = wldPosition;
        outObjNormal = normalize(objNormal);
        outWldNormal = wldNormal;
        outWldOffset = wldOffset;
    }

// Offset the world-space position along the world-space normal by the safe offset to obtain the safe spawn point.
    float3 safeSpawnPoint(
    const float3 position,
    const float3 normal,
    const float offset)
    {
        precise float3 p = mad(offset, normal, position);
        return p;
    }
}

// Calculate a tangent from triangle's vertices and their uv coordinates.
// Ref: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
float3 CalculateTangent(in float3 v0, in float3 v1, in float3 v2, in float2 uv0, in float2 uv1, in float2 uv2)
{
    // Calculate edges
    // Position delta
    float3 deltaPos1 = v1 - v0;
    float3 deltaPos2 = v2 - v0;

    // UV delta
    float2 deltaUV1 = uv1 - uv0;
    float2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    return (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
}

void Ideal_TilingAndOffset_float(float2 UV, float2 Tiling, float2 Offset, out float2 Out)
{
    Out = UV * Tiling + Offset;
}






///////////////////////////////////////////////////////////
// https://github.com/nfginola/dx11_pbr/blob/main/shaders/ForwardPBR_PS.hlsl
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float3 DirectionalLight(bool isInShadow, float3 V, float3 L, float3 N, float3 LightColor, float3 albedo, float roughness, float metallic, float intensity)
{
    if(isInShadow)
        return float3(0, 0, 0);
    
    float3 Lo = float3(0, 0, 0);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    float3 H = normalize(V + L);
    float3 radiance = LightColor * intensity;
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
    float3 kS = F;
    float3 kD = float3(1.f, 1.f, 1.f) - kS;
    kD *= 1.0 - metallic;
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float3 specular = numerator / max(denominator, 0.001);
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo;
    float3 color = ambient + Lo;
    Lo = color;
    
    return Lo;
}

float3 PointLight(bool isInShadow, float3 V, float3 Direction, float3 N, float distance, float3 LightColor, float3 albedo, float roughness, float metallic, float lightIntensity)
{
    if(isInShadow)
        return float3(0, 0, 0);
    
    float3 Lo = float3(0.f, 0.f, 0.f);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    float3 H = normalize(V + Direction);
    
    float attenuation = 1.0 / (distance * distance);
    float3 radiance = LightColor * lightIntensity* attenuation;
    
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, Direction, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
    float3 kS = F;
    float3 kD = float3(1.f, 1.f, 1.f) - kS;
    kD *= 1.0 - metallic;
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, Direction), 0.0);
    float3 specular = numerator / max(denominator, 0.001);
    float NdotL = max(dot(N, Direction), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    return Lo;
}

float3 SpotLight(bool isInShadow, float3 V, float3 Direction, float3 LightDirection, float3 N, float distance, float3 LightColor,float softness, float angle, float3 albedo, float roughness, float metallic, float lightIntensity, float range)
{
    if (isInShadow)
        return float3(0, 0, 0);
    
    
    float spotEffect = dot(normalize(-Direction), LightDirection);
    float cutoff = cos(radians(angle * 0.5));
    float SpotSoftness = softness;
    float outerCutoff = cos(radians((angle * 0.5) + SpotSoftness));
    float smoothFactor = smoothstep(outerCutoff, cutoff, spotEffect);
    float3 Lo = float3(0.f, 0.f, 0.f);
    if (spotEffect > outerCutoff)
    {
    
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, albedo, metallic);
        float3 H = normalize(V + Direction);
    
        //float attenuation = 1.0 / (distance * distance);
        float attenuation = Ideal::Attenuate(distance, range);
        float newIntensity = lightIntensity * attenuation * smoothFactor;
        float3 radiance = LightColor * newIntensity * attenuation;
    
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, Direction, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kS = F;
        float3 kD = float3(1.f, 1.f, 1.f) - kS;
        kD *= 1.0 - metallic;
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, Direction), 0.0);
        float3 specular = numerator / max(denominator, 0.001);
        float NdotL = max(dot(N, Direction), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    return Lo;
}

void Ideal_NormalStrength_float(float3 In, float Strength, out float3 Out)
{
    //Out = {precision}3(In.rg * Strength, lerp(1, In.b, saturate(Strength)));
    Out = float3(In.rg * Strength, lerp(1, In.b, saturate(Strength)));
}

float2 TriUVInfoFromRayCone(
float3 vRayDir, float3 vWorldNormal, float vRayConeWidth,
float2 aUV[3], float3 aPos[3], float3x3 matWorld
)
{
    float2 vUV10 = aUV[1] - aUV[0];
    float2 vUV20 = aUV[2] - aUV[0];
    float fTriUVArea = abs(vUV10.x * vUV20.y - vUV20.x * vUV10.y);
    
    float3 vEdge10 = mul(aPos[1] - aPos[0], matWorld);
    float3 vEdge20 = mul(aPos[2] - aPos[0], matWorld);
    float3 vFaceNrm = cross(vEdge10, vEdge20);
    
    float fTriLODOffset = 0.5f * log2(fTriUVArea / length(vFaceNrm));
    float fDistTerm = vRayConeWidth * vRayConeWidth;
    float fNormalTerm = dot(vRayDir, vWorldNormal);
    
    return float2(fTriLODOffset, fDistTerm / (fNormalTerm * fNormalTerm));
}

float TriUVInfoToTexLOD(uint2 vTexSize, float2 vUVInfo)
{
    return vUVInfo.x + 0.5f * log2(vTexSize.x * vTexSize.y * vUVInfo.y);
}

float3 GammaCorrection(float3 color, float gamma)
{
    float3 ret = pow(color, 1.0 / gamma);
    return ret;
    //return pow(color, 1.0 / gamma);
}

#endif