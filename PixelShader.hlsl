struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;
    float2 uv				: TEXTCOORD;
};

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

float4 main(VertexToPixel input) : SV_TARGET
{	
    return colorTint;
}