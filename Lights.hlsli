#ifndef __GPP_LIGHTS__
#define __GPP_LIGHTS__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

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

float GetDiffuse(Light light, float3 surfaceNormal)
{
    float3 lightDir = normalize(-light.direction);
    return saturate(dot(surfaceNormal, lightDir));
}

float GetSpecular(float3 reflection, float3 view, float roughness)
{
    // Calculate the especular exponent
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    // Make sure the exponent is above a small buffer before calculating
    if(specExponent > 0.05f)
        return pow(saturate(dot(reflection, view)), specExponent);
    else // Otherwise, return 0
        return 0.0f;
}

float GetAttenuation(Light light, float3 pixelWorldPos)
{
    float distance = distance(light.position, pixelWorldPos);
    float attenuation = saturate(1.0f - (distance * distance / (light.range * light.range)));
    return attenuation * attenuation;
}

float GetPhong(Light light, float3 cameraPos, float3 pixelWorldPos, float3 surfaceNormal, float roughness)
{
    // Get the view vector
    float3 view = normalize(cameraPos - pixelWorldPos);
    
    // Get reflection vector
    float3 reflection = reflect(light.direction, surfaceNormal);
    
    // Calculate specular for the current reflection and view vectors,
    // including roughness
    return GetSpecular(reflection, view, roughness);
}

float3 CalcDirectionalLight(Light light, float3 surfaceNormal, float3 cameraPos, float3 pixelWorldPos, float roughness)
{
    float diffuse = GetDiffuse(light, surfaceNormal);
    float phong = GetPhong(light, cameraPos, pixelWorldPos, surfaceNormal, roughness);
    
    return (diffuse + phong) * light.intensity * light.color;
}

float3 CalcPointLight(Light light, float3 surfaceNormal, float3 cameraPos, float3 pixelWorldPos, float roughness)
{
    float diffuse = GetDiffuse(light, surfaceNormal);
    float phong = GetPhong(light, cameraPos, pixelWorldPos, surfaceNormal, roughness);
    float attenuation = GetAttenuation(light, pixelWorldPos);
    
    return (diffuse + phong) * attenuation * light.intensity * light.color;
}

#endif