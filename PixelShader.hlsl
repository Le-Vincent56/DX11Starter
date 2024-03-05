#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

float4 main(VertexToPixel input) : SV_TARGET
{	
    return colorTint;
}