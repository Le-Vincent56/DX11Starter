#include "ShaderStructs.hlsli"

VertexToPixel_PP main(uint id : SV_VertexID)
{
	VertexToPixel_PP output;

	// Calculate the UV (0, 0) to (2, 2) using the ID
	output.uv = float2(
		(id << 1) & 2, // equal to id % 2 * 2
		id & 2
	);

	// Calculate the position based on the UV
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;
 
	return output;
}