struct VertexShaderInput
{ 
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;       // Normal
    float2 uv				: TEXCOORD;		// UV coordinates
};

struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
    float2 uv				: TEXCOORD;		// UV coordinate
};

cbuffer ExternalData : register(b0)
{
	matrix world;
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
	
	return output;
}