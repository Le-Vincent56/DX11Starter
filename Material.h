#pragma once
#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"

class Material
{
private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

public:
	// Constructor/Destructor
	Material(
		DirectX::XMFLOAT4 colorTint, 
		std::shared_ptr<SimplePixelShader> pixelShader,
		std::shared_ptr<SimpleVertexShader> vertexShader
	);
	~Material();

	// Getters
	DirectX::XMFLOAT4 GetColorTint() const;
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

	// Setters
	void SetColorTint(float r, float g, float b);
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
};

