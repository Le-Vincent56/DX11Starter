struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float time;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    float blink = (sin(time) * 2.0f);
    float multiplier = (blink + tan(input.uv.x)) * (sin(input.screenPosition.x) * time);
    return colorTint * multiplier;
}