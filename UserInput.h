#pragma once
#include <DirectXMath.h>
#include "Camera.h"
#include "Transform.h"

enum class ControlType
{
	None,
	Camera,
	Entity
};

class UserInput
{
public:
	UserInput(Transform& currentTarget, ControlType type);
	~UserInput();
	void SetTarget(Transform& currentTarget, ControlType type);
	void SetMovementSpeed(float movementSpeed);
	void SetLookSpeed(float lookSpeed);
	void Update(const float& deltaTime);

private:
	Transform& currentTarget;
	ControlType type;
	float movementSpeed;
	float lookSpeed;

	void UpdateCameraInput(Input& input, float currentSpeed);
};

