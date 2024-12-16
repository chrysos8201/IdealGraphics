#ifndef DisneyBSDF_H
#define DisneyBSDF_H
//https://www.shadertoy.com/view/Dtl3WS

#define saturate(x) clamp(x,0.,1.)
#define PI 3.141592653589


// ---------------------------------------------
// Hash & Random - From iq
// ---------------------------------------------

//int seed = 1;
//
//int rand()
//{
//    seed = seed * 0x343fd + 0x269ec3;
//    return (seed >> 16) & 32767;
//}
//
//float frand()
//{
//    return float(rand()) / 32767.0;
//}
//
//float2 frand2()
//{
//    return float2(frand(), frand());
//}
//
//float3 frand3()
//{
//    return float3(frand(), frand(), frand());
//}
//
//void srand(int2 p, int frame)
//{
//    int n = frame;
//    n = (n << 13) ^ n;
//    n = n * (n * n * 15731 + 789221) + 1376312589; // by Hugo Elias
//    n += p.y;
//    n = (n << 13) ^ n;
//    n = n * (n * n * 15731 + 789221) + 1376312589;
//    n += p.x;
//    n = (n << 13) ^ n;
//    n = n * (n * n * 15731 + 789221) + 1376312589;
//    seed = n;
//}

float3 hash3(float3 p)
{
    uint3 x = asuint(p);
    const uint k = 1103515245U;
    x = ((x >> 8) ^ x.yzx) * k;
    x = ((x >> 8) ^ x.yzx) * k;
    x = ((x >> 8) ^ x.yzx) * k;

    return float3(x) * (1.0 / float(0xffffffffU));
}

// func

float3 F_Schlick(float3 f0, float theta)
{
    return f0 + (1. - f0) * pow(1.0 - theta, 5.);
}

float F_Schlick(float f0, float f90, float theta)
{
    return f0 + (f90 - f0) * pow(1.0 - theta, 5.0);
}

float D_GTR(float roughness, float NoH, float k)
{
    float a2 = pow(roughness, 2.);
    return a2 / (PI * pow((NoH * NoH) * (a2 * a2 - 1.) + 1., k));
}

float SmithG(float NoV, float roughness2)
{
    float a = pow(roughness2, 2.);
    float b = pow(NoV, 2.);
    return (2. * NoV) / (NoV + sqrt(a + b - a * b));
}

float GeometryTerm(float NoL, float NoV, float roughness)
{
    float a2 = roughness * roughness;
    float G1 = SmithG(NoV, a2);
    float G2 = SmithG(NoL, a2);
    return G1 * G2;
}

float GGXVNDFPdf(float NoH, float NoV, float roughness)
{
    float D = D_GTR(roughness, NoH, 2.);
    float G1 = SmithG(NoV, roughness * roughness);
    return (D * G1) / max(0.00001, 4.0f * NoV);
}

float3 SampleGGXVNDF(float3 V, float ax, float ay, float r1, float r2)
{
    // Normalize Vh
    float3 Vh = normalize(float3(ax * V.x, ay * V.y, V.z));

    // Compute the lensq and T1
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    float3 T1 = lensq > 0.0 ? float3(-Vh.y, Vh.x, 0) * rsqrt(lensq) : float3(1, 0, 0);
    
    // Compute T2
    float3 T2 = cross(Vh, T1);

    // Generate the random values
    float r = sqrt(r1);
    float phi = 2.0 * 3.14159265358979323846 * r2; // PI constant in HLSL
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

    // Compute Nh
    float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

    // Normalize and return the result
    return normalize(float3(ax * Nh.x, ay * Nh.y, max(0.0, Nh.z)));
}

// From Pixar - https://graphics.pixar.com/library/OrthonormalB/paper.pdf
void basis(in float3 n, out float3 b1, out float3 b2)
{
    if (n.z < 0.)
    {
        float a = 1.0 / (1.0 - n.z);
        float b = n.x * n.y * a;
        b1 = float3(1.0 - n.x * n.x * a, -b, n.x);
        b2 = float3(b, n.y * n.y * a - 1.0, -n.y);
    }
    else
    {
        float a = 1.0 / (1.0 + n.z);
        float b = -n.x * n.y * a;
        b1 = float3(1.0 - n.x * n.x * a, b, -n.x);
        b2 = float3(b, 1.0 - n.y * n.y * a, -n.y);
    }
}

float3 toWorld(float3 x, float3 y, float3 z, float3 v)
{
    return v.x * x + v.y * y + v.z * z;
}

float3 toLocal(float3 x, float3 y, float3 z, float3 v)
{
    return float3(dot(v, x), dot(v, y), dot(v, z));
}

float luma(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

// From Fizzer - https://web.archive.org/web/20170610002747/http://amietia.com/lambertnotangent.html
float3 cosineSampleHemisphere(float3 n)
{
    float2 rnd = float2(3.141592, 7.473812);

    float a = PI * 2. * rnd.x;
    float b = 2.0 * rnd.y - 1.0;
    
    float3 dir = float3(sqrt(1.0 - b * b) * float2(cos(a), sin(a)), b);
    return normalize(n + dir);
}


// ---------------------------------------------
// BSDF
// ---------------------------------------------
float3 evalDisneyDiffuse(in float3 Albedo, float NoL, float NoV, float LoH, float roughness)
{
    float FD90 = 0.5 + 2. * roughness * pow(LoH, 2.);
    float a = F_Schlick(1., FD90, NoL);
    float b = F_Schlick(1., FD90, NoV);
    
    return Albedo * (a * b / PI);
}

float3 evalDisneySpecularReflection(float Roughness, float3 F, float NoH, float NoV, float NoL)
{
    float roughness = pow(Roughness, 2.);
    float D = D_GTR(roughness, NoH, 2.);
    float G = GeometryTerm(NoL, NoV, pow(0.5 + Roughness * .5, 2.));

    float3 spec = D * F * G / (4. * NoL * NoV);
    
    return spec;
}


//float4 sampleDisneyBSDF(float3 v, float3 n, in Material mat, out float3 l, inout State state)
//float4 sampleDisneyBSDF(float3 v, float3 n, in float3 Albedo, in float Metallic,  out float3 l, inout State state)
//{
//    state.hasBeenRefracted = state.isRefracted;
//    
//    float roughness = pow(mat.roughness, 2.);
//
//    // sample microfacet normal
//    float3 t, b;
//    basis(n, t, b);
//    float3 V = toLocal(t, b, n, v);
//    float3 h = SampleGGXVNDF(V, roughness, roughness, frand(), frand());
//    if (h.z < 0.0)
//        h = -h;
//    h = toWorld(t, b, n, h);
//
//    // fresnel
//    float VoH = dot(v, h);
//    float3 f0 = mix(float3(0.04), mat.albedo, mat.metallic);
//    float3 F = F_Schlick(f0, VoH);
//    float dielF = Fresnel(state.lastIOR, mat.ior, abs(VoH), 0., 1.);
//    
//    // lobe weight probability
//    float diffW = (1. - mat.metallic) * (1. - mat.specTrans);
//    float reflectW = luma(F);
//    float refractW = (1. - mat.metallic) * (mat.specTrans) * (1. - dielF);
//    float invW = 1. / (diffW + reflectW + refractW);
//    diffW *= invW;
//    reflectW *= invW;
//    refractW *= invW;
//    
//    // cdf
//    float cdf[3];
//    cdf[0] = diffW;
//    cdf[1] = cdf[0] + reflectW;
//    //cdf[2] = cdf[1] + refractW;
//    
//    
//    float4 bsdf = float4(0.);
//    float rnd = frand();
//    if (rnd < cdf[0]) // diffuse
//    {
//        l = cosineSampleHemisphere(n);
//        h = normalize(l + v);
//        
//        float NoL = dot(n, l);
//        float NoV = dot(n, v);
//        if (NoL <= 0. || NoV <= 0.)
//        {
//            return float4(0.);
//        }
//        float LoH = dot(l, h);
//        float pdf = NoL / PI;
//        
//        float3 diff = evalDisneyDiffuse(mat, NoL, NoV, LoH, roughness) * (1. - F);
//        bsdf.rgb = diff;
//        bsdf.a = diffW * pdf;
//    }
//    else if (rnd < cdf[1]) // reflection
//    {
//        l = reflect(-v, h);
//        
//        float NoL = dot(n, l);
//        float NoV = dot(n, v);
//        if (NoL <= 0. || NoV <= 0.)
//        {
//            return float4(0.);
//        }
//        float NoH = min(0.99, dot(n, h));
//        float pdf = GGXVNDFPdf(NoH, NoV, roughness);
//        
//        float3 spec = evalDisneySpecularReflection(mat, F, NoH, NoV, NoL);
//        bsdf.rgb = spec;
//        bsdf.a = reflectW * pdf;
//    }
//    else // refraction
//    {
//        state.isRefracted = !state.isRefracted;
//        float eta = state.lastIOR / mat.ior;
//        l = refract(-v, h, eta);
//        state.lastIOR = mat.ior;
//        
//        float NoL = dot(n, l);
//        if (NoL <= 0.)
//        {
//            return float4(0.);
//        }
//        float NoV = dot(n, v);
//        float NoH = min(0.99, dot(n, h));
//        float LoH = dot(l, h);
//        
//        float pdf;
//        float3 spec = evalDisneySpecularRefraction(mat, dielF, NoH, NoV, NoL, VoH, LoH, eta, pdf);
//        
//        bsdf.rgb = spec;
//        bsdf.a = refractW * pdf;
//    }
//    
//    bsdf.rgb *= abs(dot(n, l));
//
//    return bsdf;
//}

#endif