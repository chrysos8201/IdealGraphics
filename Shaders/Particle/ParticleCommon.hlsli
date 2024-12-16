    #ifndef PARTICLE_COMMON_HLSLI
    #define PARTICLE_COMMON_HLSLI

    cbuffer Global : register(b0)
    {
        float4x4 View;
        float4x4 Proj;
        float4x4 ViewProj;
        float3 eyePos;
    };

    cbuffer Transform : register(b1)
    {
        float4x4 World;
        float4x4 WorldInvTranspose;
    }
    cbuffer ParticleSystemData : register(b2)
    {
        // Time
        float4 g_CustomData1;
        float4 g_CustomData2;
        float g_Time;
        float g_currentTime;
        float2 pad0;
        float4 g_startColor;
        
        float g_DeltaTime;
        float g_MaxParticles;
        float2 g_AnimationUV_Offset;
        float2 g_AnimationUV_Scale;
    
        float2 g_ParticleSize;
        float2 pad1;

    };

    struct Pos
    {
        float4 pos;
    };

    struct ComputeParticle
    {
        float4 Position;
        float3 Direction;
        float Speed;
        float RotationAngle;
        float DelayTime;
    };
    
    StructuredBuffer<ComputeParticle> g_bufPos : register(t0);

    Texture2D ParticleTexture0 : register(t1);
    Texture2D ParticleTexture1 : register(t2);
    Texture2D ParticleTexture2 : register(t3);

    RWStructuredBuffer<ComputeParticle> g_RWBufferPos : register(u0);

    SamplerState LinearWrapSampler : register(s0);
    SamplerState LinearClampSampler : register(s1);

    struct VSInput
    {
        float3 Pos : POSITION;
        float3 Normal : NORMAL;
        float2 UV : TEXCOORD;
    };

    struct VSOutput
    {
        float4 PosH : SV_POSITION;
        float3 Pos : POSITION;
        float3 Normal : NORMAL;
        float2 UV : TEXCOORD;
    };

    struct VSParticleInput
    {
        float4 Color : COLOR;
        uint ID : SV_VERTEXID;
    };

    struct VSParticleDrawOut
    {
        float3 Pos : POSITION;
        float4 Color : COLOR;
    };

    struct GSParticleDrawOut
    {
        float4 Pos : SV_POSITION;
        float2 UV : TEXCOORD0;
        float4 Color : COLOR;
    };
    
    struct PSParticleDrawIn
    {
        //float2 UV : TEXCOORD0;
        //float4 Color : COLOR;
        float3 Pos : POSITION;
        float3 Normal : NORMAL;
        float2 UV : TEXCOORD;
    };

    cbuffer cbImmutable
    {
        static float3 g_positions[4] =
        {
            float3(-1, 1, 0),
            float3(1, 1, 0),
            float3(-1, -1, 0),
            float3(1, -1, 0),
        };
        
        static float2 g_texcoords[4] =
        {
            float2(0, 0),
            float2(1, 0),
            float2(0, 1),
            float2(1, 1),
        };
    };

    //https://docs.unity3d.com/Packages/com.unity.shadergraph@6.9/manual/Simple-Noise-Node.html
    void Ideal_TilingAndOffset_float(float2 UV, float2 Tiling, float2 Offset, out float2 Out)
    {
        Out = UV * Tiling + Offset;
    }

    //--------------------MATH-------------------//
    // Fresnel effect
    void Ideal_FresnelEffect_float(float3 Normal, float3 ViewDir, float Power, out float Out)
    {
        Out = pow((1.0 - saturate(dot(normalize(Normal), normalize(ViewDir)))), Power);
    }

    // Comparison
    void Ideal_Comparison_GreaterOrEqual_float(float A, float B, out float Out)
    {
        Out = A >= B ? 1 : 0;
    }
    // Range
        //One Minus
    void Ideal_OneMinus_float4(float4 In, out float4 Out)
    {
        Out = 1 - In;
    }

    // Remap
    void Ideal_Remap_float4(float4 In, float2 InMinMax, float2 OutMinMax, out float4 Out)
    {
        Out = OutMinMax.x + (In - InMinMax.x) * (OutMinMax.y - OutMinMax.x) / (InMinMax.y - InMinMax.x);
    }
    void Ideal_Remap_float3(float3 In, float2 InMinMax, float2 OutMinMax, out float3 Out)
    {
        Out = OutMinMax.x + (In - InMinMax.x) * (OutMinMax.y - OutMinMax.x) / (InMinMax.y - InMinMax.x);
    }   
    void Ideal_Remap_float(float In, float2 InMinMax, float2 OutMinMax, out float Out)
    {
        Out = OutMinMax.x + (In - InMinMax.x) * (OutMinMax.y - OutMinMax.x) / (InMinMax.y - InMinMax.x);
    }
    // SimpleNoise
    inline float Noise_RandomValue(float2 UV)
    {
        return frac(sin(dot(UV, float2(12.9898, 78.233))) * 43758.5453);
    }
    inline float Noise_Interpolate(float a, float b, float t)
    {
        return (1.0 - t) * a + (t * b);
    }
    inline float ValueNoise(float2 UV)
    {
        float2 i = floor(UV);
        float2 f = frac(UV);
        f = f * f * (3.0 - 2.0 * f);
    
        UV = abs(frac(UV) - 0.5);
        float2 c0 = i + float2(0.0, 0.0);
        float2 c1 = i + float2(1.0, 0.0);
        float2 c2 = i + float2(0.0, 1.0);
        float2 c3 = i + float2(1.0, 1.0);
        float r0 = Noise_RandomValue(c0);
        float r1 = Noise_RandomValue(c1);
        float r2 = Noise_RandomValue(c2);
        float r3 = Noise_RandomValue(c3);
    
        float bottomOfGrid = Noise_Interpolate(r0, r1, f.x);
        float topOfGrid = Noise_Interpolate(r2, r3, f.x);
        float t = Noise_Interpolate(bottomOfGrid, topOfGrid, f.y);
        return t;
    }

    void Ideal_SimpleNoise_float(float2 UV, float Scale, out float Out)
    {
        float t = 0.0;
        float freq = pow(2.0, float(0));
        float amp = pow(0.5, float(3 - 0));
        t += ValueNoise(float2(UV.x * Scale / freq, UV.y * Scale / freq)) * amp;
    
        freq = pow(2.0, float(1));
        amp = pow(0.5, float(3 - 1));
        t += ValueNoise(float2(UV.x * Scale / freq, UV.y * Scale / freq)) * amp;
    
        freq = pow(2.0, float(2));
        amp = pow(0.5, float(3 - 2));
        t += ValueNoise(float2(UV.x * Scale / freq, UV.y * Scale / freq)) * amp;
    
        Out = t;
    }
    // Gradient Noise
    float2 GradientNoise_dir(float2 p)
    {
        p = p % 289;
        float x = (34 * p.x + 1) * p.x % 289 + p.y;
        x = (34 * x + 1) * x % 289;
        x = frac(x / 41) * 2 - 1;
        return normalize(float2(x - floor(x + 0.5), abs(x) - 0.5));
    }
    
    float GradientNoise(float2 p)
    {
        float2 ip = floor(p);
        float2 fp = frac(p);
        float d00 = dot(GradientNoise_dir(ip), fp);
        float d01 = dot(GradientNoise_dir(ip + float2(0, 1)), fp - float2(0, 1));
        float d10 = dot(GradientNoise_dir(ip + float2(1, 0)), fp - float2(1, 0));
        float d11 = dot(GradientNoise_dir(ip + float2(1, 1)), fp - float2(1, 1));
        fp = fp * fp * fp * (fp * (fp * 6 - 15) + 10);
        return lerp(lerp(d00, d01, fp.y), lerp(d10, d11, fp.y), fp.x);
    }
    
    void Ideal_GradientNoise_float(float2 UV, float Scale, out float Out)
    {
        Out = GradientNoise(UV * Scale) + 0.5;
    }

    // Round // Step

    void Step_float4(float4 Edge, float4 In, out float4 Out)
    {
        Out = step(Edge, In);
    }
    
    // Artistic
        // Adjustment
            // Hue
    void Ideal_Hue_Degrees_float(float3 In, float Offset, out float3 Out)
    {
        float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        float4 P = lerp(float4(In.bg, K.wz), float4(In.gb, K.xy), step(In.b, In.g));
        float4 Q = lerp(float4(P.xyw, In.r), float4(In.r, P.yzx), step(P.x, In.r));
        float D = Q.x - min(Q.w, Q.y);
        float E = 1e-10;
        float3 hsv = float3(abs(Q.z + (Q.w - Q.y) / (6.0 * D + E)), D / (Q.x + E), Q.x);
        
        float hue = hsv.x + Offset / 360;
        hsv.x = (hue < 0) ? hue + 1
                            : (hue > 1) ? hue - 1
                                        : hue;
        float4 K2 = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 P2 = abs(frac(hsv.xxx + K2.xyz) * 6.0 - K2.www);
        Out = hsv.z * lerp(K2.xxx, saturate(P2 - K2.xxx), hsv.y);
    }
    
    void Ideal_Hue_Radians_float(float3 In, float Offset, out float3 Out)
    {
        float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        float4 P = lerp(float4(In.bg, K.wz), float4(In.gb, K.xy), step(In.b, In.g));
        float4 Q = lerp(float4(P.xyw, In.r), float4(In.r, P.yzx), step(P.x, In.r));
        float D = Q.x - min(Q.w, Q.y);
        float E = 1e-10;
        float3 hsv = float3(abs(Q.z + (Q.w - Q.y) / (6.0 * D + E)), D / (Q.x + E), Q.x);
    
        float hue = hsv.x + Offset;
        hsv.x = (hue < 0)
                ? hue + 1
                : (hue > 1)
                    ? hue - 1
                    : hue;
    
        // HSV to RGB
        float4 K2 = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 P2 = abs(frac(hsv.xxx + K2.xyz) * 6.0 - K2.www);
        Out = hsv.z * lerp(K2.xxx, saturate(P2 - K2.xxx), hsv.y);
    }

    // Artistic
        // Adjustment
            // Saturation
    void Ideal_Saturation_float(float3 In, float Saturation, out float3 Out)
    {
        float luma = dot(In, float3(0.2126729, 0.7151522, 0.0721750));
        Out = luma.xxx + Saturation.xxx * (In - luma.xxx);
    }

    // Procedural
           // Utility
               // Logic
                   // Branch
    void Ideal_Branch_float4(float Predicate, float4 True, float4 False, out float4 Out)
    {
        Out = Predicate ? True : False;
    }
    void Ideal_Branch_float(float Predicate, float True, float False, out float Out)
    {
        Out = Predicate ? True : False;
    }
    #endif  
