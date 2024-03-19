#pragma once
#include <memory>
#include <DirectXMath.h>

#include "SimpleShader.h"
#include "Lights.h"

class LightManager
{
private:
	DirectX::XMFLOAT3 ambientTerm;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::vector<Light> lights;
	std::vector<LightData> lightDatas;

public:
	// Constructor/Destructor
	LightManager(std::shared_ptr<SimplePixelShader> pixelShader);
	~LightManager();

	// Setters
	void SetAmbientTerm(DirectX::XMFLOAT3 ambientTerm);
	void SetLights(std::vector<Light> lights);

	// Getters
	DirectX::XMFLOAT3 GetAmbientTerm() const;
	std::vector<Light> GetLights();

	void Init();
	void SetPixelData();
	void UpdateLightData();
};

