#include "UserInput.h"
#include "MathUtils.h"

using namespace DirectX;

UserInput::UserInput(Transform& currentTarget, ControlType type) : 
    currentTarget(currentTarget), 
    type(type)
{
	movementSpeed = 0.0f;
	lookSpeed = 0.0f;
}

UserInput::~UserInput()
{
}

void UserInput::SetTarget(Transform& currentTarget, ControlType type)
{
	this->currentTarget = currentTarget;
    this->type = type;
}

void UserInput::SetMovementSpeed(float movementSpeed)
{
	this->movementSpeed = movementSpeed;
}

void UserInput::SetLookSpeed(float lookSpeed)
{
	this->lookSpeed = lookSpeed;
}

void UserInput::Update(const float& deltaTime)
{
    // Update movement speed
    float currentSpeed = movementSpeed * deltaTime;

    // Get input
    Input& input = Input::GetInstance();

    // Update user input depending on the current control type
	switch (type)
	{
	case ControlType::Camera:
		UpdateCameraInput(input, currentSpeed);
		break;

	case ControlType::Entity:
        // TODO: Move entities in the future, if necessary
		break;
	}
}

void UserInput::UpdateCameraInput(Input& input, float currentSpeed)
{
    // Speed up or down as necessary
    float modifier = 1.0f;
    if (input.KeyDown(VK_SHIFT)) { modifier = 2; } // Speed up by 2x using Shift
    if (input.KeyDown(VK_MENU)) { modifier = 0.5f; } // Slow down by 0.5x using Alt

    // Multiply by modifier
    currentSpeed *= modifier;

    // Get relative movement from input
    XMFLOAT3 relativeInput(0, 0, 0);
    if (input.KeyDown('W')) { relativeInput.z += currentSpeed; } // Move forward
    if (input.KeyDown('S')) { relativeInput.z += -currentSpeed; } // Move backward
    if (input.KeyDown('A')) { relativeInput.x += -currentSpeed; } // Move left
    if (input.KeyDown('D')) { relativeInput.x += currentSpeed; } // Move right

    // Perform relative move
    currentTarget.MoveRelative(relativeInput);

    // Get absolute movement from input
    XMFLOAT3 absoluteInput(0, 0, 0);
    if (input.KeyDown(VK_SPACE)) { absoluteInput.y += currentSpeed; } // Move up
    if (input.KeyDown(VK_CONTROL)) { absoluteInput.y -= currentSpeed; } // Move down

    // Perform absolute move
    currentTarget.MoveAbsolute(absoluteInput);

    // Check if the left mouse button is being held down
    if (input.MouseLeftDown())
    {
        // Get the mouse deltas and multiply by the look speed
        float xDelt = input.GetMouseXDelta() * lookSpeed;
        float yDelt = input.GetMouseYDelta() * lookSpeed;

        // Rotate by the deltas in opposite order
        currentTarget.Rotate(yDelt, xDelt, 0);

        // Clamp the x rotation
        XMFLOAT3 currentRotation = currentTarget.GetPitchYawRoll();
        currentRotation.x = MathUtils::Clamp(currentRotation.x, XM_PIDIV2, -XM_PIDIV2);
        currentTarget.SetRotation(currentRotation);
    }
}
