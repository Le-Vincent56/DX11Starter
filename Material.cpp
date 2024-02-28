#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint, 
    std::shared_ptr<SimplePixelShader> pixelShader, 
    std::shared_ptr<SimpleVertexShader> vertexShader)
    :
    colorTint(colorTint),
    pixelShader(pixelShader),
    vertexShader(vertexShader)
{
}

Material::~Material()
{
}

DirectX::XMFLOAT4 Material::GetColorTint() const
{
    return this->colorTint;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
    return this->pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
    return this->vertexShader;
}

void Material::SetColorTint(float r, float g, float b)
{
    // Set component values
    colorTint.x = r;
    colorTint.y = g;
    colorTint.z = b;
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
    // Call component version using parameter components
    SetColorTint(colorTint.x, colorTint.y, colorTint.z);
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader)
{
    this->pixelShader = pixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader)
{
    this->vertexShader = vertexShader;
}
