#ifndef __GPP_SHADER_INCLUDES__
#define __GPP_SHADER_INCLUDES__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float2 uv : TEXCOORD; // UV coordinate
    float3 normal : NORMAL; // Normals
    float3 worldPosition : POSITION; // World position
};

struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotFallOff;
    float3 padding;
};

#endif