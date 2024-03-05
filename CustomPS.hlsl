#include "ShaderStructs.hlsli"

cbuffer EntityData : register(b0)
{
    float3 colorTint;
    float roughness;
    float3 ambientTerm;
    float time;
    
}

cbuffer FrameData : register(b1)
{
    float3 cameraPos;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    // Normalize input normals
    input.normal = normalize(input.normal);

    float blink = (sin(time) * 2.0f);
    float multiplier = (blink + tan(input.uv.x)) * (sin(input.screenPosition.x) * time);
    float3 ambientColor = colorTint * ambientTerm;
    return float4(ambientColor, 1);
}