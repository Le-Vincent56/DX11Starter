#include "ShaderStructs.hlsli"
#include "Lights.hlsli"

Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);
SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

cbuffer EntityData : register(b0)
{
    float3 colorTint;
    float roughness;
    float3 ambientTerm;
    float time;
    float offset;
    float scale;
    Light lights[5];
}

cbuffer FrameData : register(b1)
{
    float3 cameraPos;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    // Divide by w to perform the perspective divide
    input.shadowMapPos /= input.shadowMapPos.w;
    
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    
    // Flip the y value
    shadowUV.y = 1 - shadowUV.y;
    
    // Get distances: light-to-pixel and closest-surface
    float distToLight = input.shadowMapPos.z;
    
    // Get a ratio of comparison results
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distToLight).r;
    
    // Normalize input normals
    input.normal = normalize(input.normal);

    // Edit input UV's depending on material
    //float2 uv = (input.uv.x + (offset * time)) * scale;
    
    // Sample and unpack normals
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    // Normalize tangent
    input.tangent = normalize(input.tangent);
    
    // Create the TBN matrix
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - N * dot(T, N));
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    // Transform the normal using the normal map
    input.normal = mul(unpackedNormal, TBN);

    // Get surface color of the texture
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    surfaceColor *= colorTint;

    // Sample roughness
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;

    // Sample metalness
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

    // Calculate specular color
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

    // Get the total color
    float3 totalColor = surfaceColor;

    // Normalize lighting
    for (int i = 0; i < 3; i++)
    {
        Light light = lights[i];

        // Normalize the light direction
        light.direction = normalize(light.direction);

        // Add directional light
        switch (light.type)
        {
            // Directional Light
            case LIGHT_TYPE_DIRECTIONAL:
                float3 lightResult = CalcDirectionalLight(light, input.normal, cameraPos, input.worldPosition, 
                    roughness, metalness, surfaceColor, specularColor);
            
                if (i == 0)
                {
                    lightResult *= shadowAmount;
                }
            
                totalColor += lightResult;
                break;
            
            // Point Light
            case LIGHT_TYPE_POINT:
                totalColor += CalcPointLight(light, input.normal, cameraPos, input.worldPosition, 
                    roughness, metalness, surfaceColor, specularColor);
                break;
            
            // Spot Light
            case LIGHT_TYPE_SPOT:
                // TODO
                break;
        }
        
    }

    return float4(pow(totalColor, 1.0f / 2.2f), 1);
}