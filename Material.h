#pragma once
#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"
#include "Transform.h"

class Material
{
private:
	// Shaders
	DirectX::XMFLOAT3 colorTint;
	float roughness;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	// Textures
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> textureSamplers;

public:
	// Constructor/Destructor
	Material(
		DirectX::XMFLOAT3 colorTint, 
		float roughness,
		std::shared_ptr<SimplePixelShader> pixelShader,
		std::shared_ptr<SimpleVertexShader> vertexShader
	);
	~Material();

	// Getters
	DirectX::XMFLOAT3 GetColorTint() const;
	float GetRoughness();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

	// Setters
	void SetColorTint(float r, float g, float b);
	void SetColorTint(DirectX::XMFLOAT3 colorTint);
	void SetRoughness(float roughness);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void AddTextureSRV(std::string key, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> value);
	void AddSamplerState(std::string key, Microsoft::WRL::ComPtr<ID3D11SamplerState> value);

	// Functions
	void PrepareMaterial(Transform* transform, DirectX::XMFLOAT3 ambientTerm, float totalTime);
};

