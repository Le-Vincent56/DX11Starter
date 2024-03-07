#include "LightManager.h"

using namespace DirectX;

LightManager::LightManager(std::shared_ptr<SimplePixelShader> pixelShader)
	: pixelShader(pixelShader)
{
	// Initialize lights
	Init();
}

LightManager::~LightManager()
{
}

void LightManager::Init()
{
	// Create the first light
	Light light1 = {};
	light1.Type = LIGHT_TYPE_DIRECTIONAL;
	light1.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	light1.Color = XMFLOAT3(0.2f, 0.6f, 1.0f);
	light1.Intensity = 0.6f;

	// Create the second light
	Light light2 = {};
	light2.Type = LIGHT_TYPE_DIRECTIONAL;
	light2.Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	light2.Color = XMFLOAT3(1.0f, 0.2f, 0.6f);
	light2.Intensity = 0.8f;

	// Create the third light
	Light light3 = {};
	light3.Type = LIGHT_TYPE_DIRECTIONAL;
	light3.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	light3.Color = XMFLOAT3(1.0f, 0.2f, 0.6f);
	light3.Intensity = 0.3f;

	// Add the lights
	lights.push_back(light1);
	lights.push_back(light2);
	lights.push_back(light3);
}

void LightManager::SetPixelData()
{
	// Set pixel shader data
	this->pixelShader->SetData(
		"lights",
		&lights[0],
		(sizeof(Light) * (int)lights.size())
	);
}


