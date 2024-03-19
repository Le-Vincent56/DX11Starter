#include "Material.h"

Material::Material(
    DirectX::XMFLOAT3 colorTint,
    float roughness,
    std::shared_ptr<SimplePixelShader> pixelShader, 
    std::shared_ptr<SimpleVertexShader> vertexShader)
    :
    colorTint(colorTint),
    roughness(roughness),
    pixelShader(pixelShader),
    vertexShader(vertexShader)
{
}

Material::~Material()
{
}

DirectX::XMFLOAT3 Material::GetColorTint() const
{
    return this->colorTint;
}

float Material::GetRoughness()
{
    return this->roughness;
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

void Material::SetColorTint(DirectX::XMFLOAT3 colorTint)
{
    // Call component version using parameter components
    SetColorTint(colorTint.x, colorTint.y, colorTint.z);
}

void Material::SetRoughness(float roughness)
{
    this->roughness = roughness;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader)
{
    this->pixelShader = pixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader)
{
    this->vertexShader = vertexShader;
}

// --------------------------------------------------------
// Add a ShaderResourceView to the map associated with a string
// --------------------------------------------------------
void Material::AddTextureSRV(std::string key, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> value)
{
    textureSRVs.insert({ key, value });
}


// --------------------------------------------------------
// Add a SamplerState to the map associated with a string
// --------------------------------------------------------
void Material::AddSamplerState(std::string key, Microsoft::WRL::ComPtr<ID3D11SamplerState> value)
{
    textureSamplers.insert({ key, value });
}

void Material::PrepareMaterial(Transform* transform, DirectX::XMFLOAT3 ambientTerm, float totalTime)
{
    // Set shaders
    pixelShader->SetShader();
    vertexShader->SetShader();

    // Update textures
    for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
    for (auto& s : textureSamplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }

    // Update pixel shader info for each entity
    pixelShader->SetFloat3("colorTint", colorTint);
    pixelShader->SetFloat("time", totalTime * 5.0f);
    pixelShader->SetFloat3("ambientTerm", ambientTerm);
    pixelShader->SetFloat("roughness", roughness);

    // Update vertex shader info for each entity
    vertexShader->SetMatrix4x4("world", transform->GetWorldMatrix());
    vertexShader->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());

    pixelShader->CopyBufferData("EntityData");
    vertexShader->CopyBufferData("EntityData");
}
