#include "Lights.h"

Light::Light()
{
	// Initialize the light to an empty struct
	data = {};
}

void Light::SetType(int type)
{
	this->data.Type = type;
}

void Light::SetDirection(DirectX::XMFLOAT3 direction)
{
	this->data.Direction = direction;
}

void Light::SetRange(float range)
{
	this->data.Range = range;
}

void Light::SetPosition(DirectX::XMFLOAT3 position)
{
	// Update both transform and data positions
	this->transform.SetPosition(position);
	this->data.Position = transform.GetPosition();
}

void Light::SetIntensity(float intensity)
{
	this->data.Intensity = intensity;
}

void Light::SetColor(DirectX::XMFLOAT3 color)
{
	this->data.Color = color;
}

void Light::SetSpotFallOff(float spotFallOff)
{
	this->data.SpotFallOff = spotFallOff;
}

Transform* Light::GetTransform()
{
	return &this->transform;
}

LightData Light::GetData() const
{
	return this->data;
}

int Light::GetType() const
{
	return this->data.Type;
}

DirectX::XMFLOAT3 Light::GetDirection() const
{
	return this->data.Direction;
}

float Light::GetRange() const
{
	return this->data.Range;
}

DirectX::XMFLOAT3 Light::GetPosition() const
{
	return this->data.Position;
}

float Light::GetIntensity() const
{
	return this->data.Intensity;
}

DirectX::XMFLOAT3 Light::GetColor() const
{
	return this->data.Color;
}

float Light::GetSpotFallOff() const
{
	return this->data.SpotFallOff;
}
