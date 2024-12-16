#ifndef TEST_CUSTOM_PARTICLE
#define TEST_CUSTOM_PARTICLE

#include "ParticleCommon.hlsli" // necessary

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 result = float4(0,0,0,0);
    {
        float2 emissionTiling;
        Ideal_TilingAndOffset_float(input.UV, float2(1.2,1), float2(0,0), emissionTiling);
        float4 SampleEmissionTex = ParticleTexture1.Sample(LinearClampSampler, emissionTiling);
        
        float3 hueOut0;
        Ideal_Hue_Radians_float(SampleEmissionTex.xyz, g_CustomData2.z, hueOut0);
        //Ideal_Hue_Radians_float(SampleEmissionTex.xyz, 0.3, hueOut0);
            
        float3 saturation0;
        float desaturation0 = 0;
        Ideal_Saturation_float(hueOut0, 1 - desaturation0, saturation0);
        float2 RemapData = float2(-0.3, 1);
        //float2 RemapData = float2(-2, 1);
        float3 remap0;
        Ideal_Remap_float3(saturation0, float2(0,1), RemapData, remap0);
        float3 clamp0 = clamp(remap0, float3(0,0,0), float3(1,1,1));
        
        float EmissionData = 4.79;
        float3 mul0 = clamp0 * EmissionData;
        
        float4 AddColorData = float4(0.0113418, 0, 0.1981132, 0);
        //float4 AddColorData = float4(0.1037736, 0.07531176, 0.06020825, 0);
        float4 mul1 = g_startColor * AddColorData;
        float3 mul2 = g_startColor.xyz * mul0;
        float3 add0 = mul1.xyz + mul2;
        result.xyz = add0;
    }
    float4 SpeedMainTexUVPlusNoise = float4(0,0,0.8,0);
    {
        float2 tiling0;
        float2 tiling1;
        Ideal_TilingAndOffset_float(input.UV, float2(1,1), float2(0,0), tiling0);
        Ideal_TilingAndOffset_float(tiling0, float2(1.2, 1), float2(0,0), tiling1);
        float4 SampleMainTex = ParticleTexture0.Sample(LinearClampSampler, tiling1);
        float OpacityData = 20;
        float newW = SampleMainTex.x * OpacityData;
        //result.w = SampleMainTex.x * OpacityData;
        float saturate0 = saturate(newW);
        float block0;
        {
            float2 mul0 = SpeedMainTexUVPlusNoise.zw * g_Time;
            //float2 mul0 = float2(0,0);
            float2 tiling1_0;
            float2 tiling1_1;
            Ideal_TilingAndOffset_float(input.UV, float2(1,1), mul0, tiling1_0);
            tiling1_0.y += g_CustomData2.w;
            Ideal_TilingAndOffset_float(tiling1_0, float2(1,2), float2(0,0), tiling1_1);
            //float4 SampleDissolveTex = ParticleTexture2.Sample(LinearClampSampler, tiling1_1);
            float4 SampleDissolveTex = ParticleTexture2.Sample(LinearWrapSampler, tiling1_1);
            float comp0;
            {
                float param0 = 1 - input.UV.x;
                float step1_1 = step(param0, g_CustomData1.y);
                //float branch1_0;
                //Ideal_Branch_float(0, 1, g_CustomData1.x, branch1_0);
                comp0 = step1_1 * g_CustomData1.x;
            }
            float compOut;
            Ideal_Comparison_GreaterOrEqual_float(comp0, SampleDissolveTex.x, compOut);
            //result.xyz = float3(compOut, compOut, compOut);
            //result.w = 1;
            float branchOut0;   
            Ideal_Branch_float(compOut, 1, 0, branchOut0);

            float dissolveX = SampleDissolveTex.x + 0.5;
            dissolveX = dissolveX - g_CustomData1.x;
            
            float satOut = saturate(dissolveX);
            //satOut = satOut * branchOut0;
            satOut = satOut * 0;
            Ideal_Branch_float(0, satOut, branchOut0, block0);
            //block0 = branchOut0;
            //result.w = satOut;
        }
        float block1 = saturate0 * block0;
        float block1_0 = g_startColor.w * block1;
        result.w = block1_0;
        //result.w = saturate0;
        //result.w = g_startColor.w;
    }
    
    return result;
}


#endif
