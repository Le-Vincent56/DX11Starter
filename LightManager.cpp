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
	// Initialize an empty Light struct
	light = {};

	// Fill light struct
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	light.Color = XMFLOAT3(0.2f, 0.2f, 1.0f);
	light.Intensity = 1.0f;
}

void LightManager::SetPixelData()
{
	// Set pixel shader data
	this->pixelShader->SetData(
		"light",
		&light,
		sizeof(Light)
	);
}


