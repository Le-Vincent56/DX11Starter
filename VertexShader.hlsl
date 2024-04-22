#include "ShaderStructs.hlsli"

cbuffer EntityData : register(b0)
{
	matrix world;
	matrix worldInvTranspose;
	
    matrix lightView;
    matrix lightProjection;
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
	
    matrix shadowWVP = mul(lightProjection, mul(lightView, world));
    output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));
	
	return output;
}