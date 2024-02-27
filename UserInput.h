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
	UserInput();
	~UserInput();
	void SetTarget(Transform* currentTarget, ControlType type);
	void SetMovementSpeed(float movementSpeed);
	void SetLookSpeed(float lookSpeed);

private:
	Transform* currentTarget;
	ControlType type;
	Camera* camera;
	float movementSpeed;
	float lookSpeed;

	void Update(float& dt);
	void UpdateCameraInput(Input& input, float currentSpeed);
};

