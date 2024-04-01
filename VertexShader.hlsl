#include "ShaderStructs.hlsli"

struct VertexShaderInput
{ 
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;       // Normal
    float3 tangent			: TANGENT;
    float2 uv				: TEXCOORD;		// UV coordinates
};

cbuffer EntityData : register(b0)
{
	matrix world;
	matrix worldInvTranspose;
}

cbuffer FrameData : register(b1)
{
	matrix view;
	matrix projection;
}

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;
	
	// Get the screen position of the vertex
	matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Set UV and Normals
    output.uv = input.uv;
	output.normal = mul((float3x3)worldInvTranspose, input.normal);
    output.tangent = mul((float3x3)world, input.tangent);
	
	// Set world position
	output.worldPosition = mul(world, float4(input.localPosition, 1)).xyz;
	
	return output;
}