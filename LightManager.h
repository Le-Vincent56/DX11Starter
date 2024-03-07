#pragma once
#include <memory>

#include "SimpleShader.h"
#include "Lights.h"

class LightManager
{
private:
	std::shared_ptr<SimplePixelShader> pixelShader;

public:
	// Constructor/Destructor
	LightManager(std::shared_ptr<SimplePixelShader> pixelShader);
	~LightManager();

	void Init();
	void SetPixelData();

	std::vector<Light> lights;
};

