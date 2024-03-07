#include "ShaderStructs.hlsli"
#include "Lights.hlsli"

cbuffer EntityData : register(b0)
{
    float3 colorTint;
    float roughness;
    float3 ambientTerm;
    float time;
    Light lights[3];
}

cbuffer FrameData : register(b1)
{
    float3 cameraPos;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    // Normalize input normals
    input.normal = normalize(input.normal);

    /*float blink = (sin(time) * 2.0f);
    float multiplier = (blink + tan(input.uv.x)) * (sin(input.screenPosition.x) * time);
    float3 ambientColor = colorTint * ambientTerm;*/

    // Get the total color
    float3 totalColor = colorTint * ambientTerm;

    // Normalize lighting
    for (int i = 0; i < 3; i++)
    {
        Light light = lights[i];
        light.direction = normalize(light.direction);

        // Add directional light
        totalColor += CalcDirectionalLight(light, input.normal, cameraPos, input.worldPosition, roughness);
    }

    return float4(totalColor, 1);
}