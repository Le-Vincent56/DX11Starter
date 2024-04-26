#include "ShaderStructs.hlsli"

cbuffer externalData : register(b0)
{
    float pixelSize;
    float2 textureSize;
}

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel_PP input) : SV_TARGET
{
    // Get the default texture
    float4 pixel = Pixels.Sample(ClampSampler, input.uv);
    
    // If disabled, then return the normal texture
    if(pixel.a <= 0.0)
    {
        return pixel;
    }
    
    // Calculate pixelation
    float x = floor(input.uv.x * textureSize.x / pixelSize) * pixelSize + (pixelSize / 2.0);
    float y = floor(input.uv.y * textureSize.y / pixelSize) * pixelSize + (pixelSize / 2.0);

    float2 pixelatedUV = float2(x, y) / textureSize;
    
    float4 output = Pixels.Sample(ClampSampler, pixelatedUV);
    
    return output;
}