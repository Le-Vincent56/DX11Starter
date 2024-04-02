#ifndef __GPP_SHADER_INCLUDES__
#define __GPP_SHADER_INCLUDES__

struct VertexShaderInput
{
    float3 localPosition : POSITION; // XYZ position
    float3 normal : NORMAL; // Normal
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD; // UV coordinates
};

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float2 uv : TEXCOORD; // UV coordinate
    float3 normal : NORMAL; // Normals
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION; // World position
};

struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

#endif