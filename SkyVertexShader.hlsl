#include "ShaderStructs.hlsli"

cbuffer SkyData : register(b0)
{
    matrix view;
    matrix projection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
    // Create an output variable
    VertexToPixel_Sky output;
    
    // Copy the view matrix and set the translation to zero
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    // Apply projection and updated view to the input position
    // Cast local position to 4 values to include w
    output.position = mul(mul(projection, viewNoTranslation), float4(input.localPosition, 1.0f));
    
    // Set the position's Z value equal to the position's W value so the division will ALWAYS
    // equal 1.0
    output.position.z = output.position.w;
    
    // Set the sample direction equal to the input position
    output.sampleDir = input.localPosition;
    
    // Return the output
    return output;
}