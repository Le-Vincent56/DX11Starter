#ifndef __GPP_SHADER_INCLUDES__
#define __GPP_SHADER_INCLUDES__

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float2 uv : TEXCOORD; // UV coordinate
    float3 normal : NORMAL; // Normals
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION; // World position
};

#endif