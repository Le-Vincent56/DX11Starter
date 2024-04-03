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

void LightManager::SetAmbientTerm(DirectX::XMFLOAT3 ambientTerm)
{
	this->ambientTerm = ambientTerm;
}

void LightManager::SetLights(std::vector<Light> lights)
{
	this->lights = lights;

	// Update light data
	UpdateLightData();
}

DirectX::XMFLOAT3 LightManager::GetAmbientTerm() const
{
	return this->ambientTerm;
}

std::vector<Light> LightManager::GetLights()
{
	return this->lights;
}

void LightManager::Init()
{
	// Set ambient term
	ambientTerm = XMFLOAT3(0.05f, 0.15f, 0.25f);

	// Create the first light
	Light light1;
	light1.SetType(LIGHT_TYPE_DIRECTIONAL);
	light1.SetDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	light1.SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	light1.SetIntensity(0.3f);

	// Create the second light
	Light light2;
	light2.SetType(LIGHT_TYPE_DIRECTIONAL);
	light2.SetDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	light2.SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	light2.SetIntensity(0.6f);

	// Create the third light
	Light light3;
	light3.SetType(LIGHT_TYPE_DIRECTIONAL);
	light3.SetDirection(XMFLOAT3(0.0f, -1.0f, 0.0f));
	light3.SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	light3.SetIntensity(0.5f);

	// Create a fourth light
	Light light4;
	light4.SetType(LIGHT_TYPE_POINT);
	light4.SetDirection(XMFLOAT3(0.0f, -1.0f, 0.0f));
	light4.SetRange(24.0f);
	light4.SetPosition(XMFLOAT3(0.0f, 10.0f, 0.0f));
	light4.SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	light4.SetIntensity(0.7f);

	Light light5;
	light5.SetType(LIGHT_TYPE_POINT);
	light5.SetDirection(XMFLOAT3(0.5f, 0.5f, 0.0f));
	light5.SetRange(10.0f);
	light5.SetPosition(XMFLOAT3(-3.5f, -3.5f, 0.0f));
	light5.SetColor(XMFLOAT3(1.0f, 1.0f, 1.0f));
	light5.SetIntensity(0.7f);

	// Add the lights
	lights.push_back(light1);
	lights.push_back(light2);
	lights.push_back(light3);
	lights.push_back(light4);
	lights.push_back(light5);

	// Add datas, making sure that indices match up
	for (int i = 0; i < lights.size(); ++i)
	{
		lightDatas.push_back(lights[i].GetData());
	}
}

void LightManager::SetPixelData()
{
	// Set pixel shader data
	this->pixelShader->SetData(
		"lights",
		&lightDatas[0],
		(sizeof(LightData) * (int)lights.size())
	);
}

void LightManager::UpdateLightData()
{
	// Update light data
	for (int i = 0; i < lights.size(); i++)
	{
		lightDatas[i] = lights[i].GetData();
	}
}

