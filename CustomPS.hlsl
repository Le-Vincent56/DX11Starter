#include "ShaderStructs.hlsli"
#include "Lights.hlsli"

Texture2D SurfaceTexture : register(t0);
SamplerState BasicSampler : register(s0);

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
    // Normalize input normals
    input.normal = normalize(input.normal);

    // Edit input UV's depending on material
    float2 uv = (input.uv.x + (offset * time)) * scale;

    // Get surface color of the texture
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, uv).rgb;

    // Get the total color
    float3 totalColor = surfaceColor * colorTint * ambientTerm;

    // Normalize lighting
    for (int i = 0; i < 5; i++)
    {
        Light light = lights[i];
        light.direction = normalize(light.direction);

        // Add directional light
        switch (light.type)
        {
            // Directional Light
            case LIGHT_TYPE_DIRECTIONAL:
                totalColor += CalcDirectionalLight(light, input.normal, cameraPos, input.worldPosition, roughness);
                break;
            
            // Point Light
            case LIGHT_TYPE_POINT:
                totalColor += CalcPointLight(light, input.normal, cameraPos, input.worldPosition, roughness);
                break;
            
            // Spot Light
            case LIGHT_TYPE_SPOT:
                // TODO
                break;
        }
        
    }

    return float4(totalColor, 1);
}