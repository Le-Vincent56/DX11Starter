#include "ShaderStructs.hlsli"

TextureCube CubeMap : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    // Sample the cube map in the correct direction
    return CubeMap.Sample(BasicSampler, input.sampleDir);
}