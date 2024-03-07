#pragma once
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#include <DirectXMath.h>
#include "Transform.h"

struct LightData
{
	int Type;
	DirectX::XMFLOAT3 Direction;
	float Range;
	DirectX::XMFLOAT3 Position;
	float Intensity;
	DirectX::XMFLOAT3 Color;
	float SpotFallOff;
	DirectX::XMFLOAT3 Padding;
};

class Light 
{
private:
	LightData data;
	Transform transform;

public:
	Light();

	// Setters
	void SetType(int type);
	void SetDirection(DirectX::XMFLOAT3 direction);
	void SetRange(float range);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetIntensity(float intensity);
	void SetColor(DirectX::XMFLOAT3 color);
	void SetSpotFallOff(float spotFallOff);

	// Getters
	Transform* GetTransform();
	LightData GetData() const;
	int GetType() const;
	DirectX::XMFLOAT3 GetDirection() const;
	float GetRange() const;
	DirectX::XMFLOAT3 GetPosition() const;
	float GetIntensity() const;
	DirectX::XMFLOAT3 GetColor() const;
	float GetSpotFallOff() const;
};
