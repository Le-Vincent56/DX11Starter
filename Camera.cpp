#include "Camera.h"

using namespace DirectX;

Camera::Camera(
    DirectX::XMFLOAT3 position, 
    float movementSpeed, 
    float mouseLookSpeed, 
    float fieldOfView, 
    float aspectRatio, 
    float nearClip, 
    float farClip, 
    ProjectionType projectionType)
    :
    movementSpeed(movementSpeed),
    mouseLookSpeed(mouseLookSpeed),
    fieldOfView(fieldOfView),
    aspectRatio(aspectRatio),
    nearClip(nearClip),
    farClip(farClip),
    projectionType(projectionType)
{
    // Set the transform position
    transform.SetPosition(position);

    // Update camera matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::Camera(
    float x, 
    float y, 
    float z, 
    float movementSpeed, 
    float mouseLookSpeed, 
    float fieldOfView, 
    float aspectRatio, 
    float nearClip, 
    float farClip, 
    ProjectionType projectionType)
{
    // Set the transform position
    transform.SetPosition(x, y, z);

    // Update camera matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::SetMovementSpeed(float movementSpeed)
{
    this->movementSpeed = movementSpeed;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetMouseLookSpeed(float mouseLookSpeed)
{
    this->mouseLookSpeed = mouseLookSpeed;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetFieldOfView(float fieldOfView)
{
    this->fieldOfView = fieldOfView;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetNearClip(float nearClip)
{
    this->nearClip = nearClip;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetFarClip(float farClip)
{
    this->farClip = farClip;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetOrthographicWidth(float orthographicWidth)
{
    this->orthographicWidth = orthographicWidth;
    UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetProjectionType(ProjectionType projectionType)
{
    this->projectionType = projectionType;
    UpdateProjectionMatrix(aspectRatio);
}

Transform* Camera::GetTransform()
{
    return &this->transform;
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
    return this->viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
    return this->projMatrix;
}

float Camera::GetMovementSpeed()
{
    return this->movementSpeed;
}

float Camera::GetMouseLookSpeed()
{
    return this->mouseLookSpeed;
}

float Camera::GetFieldOfView()
{
    return this->fieldOfView;
}

float Camera::GetAspectRatio()
{
    return this->aspectRatio;
}

float Camera::GetNearClip()
{
    return this->nearClip;
}

float Camera::GetFarClip()
{
    return this->farClip;
}

float Camera::GetOrthographicWidth()
{
    return this->orthographicWidth;
}

ProjectionType Camera::GetProjectionType()
{
    return this->projectionType;
}

void Camera::Update(float dt)
{
    // Get the current speed - allows the speed to be transformed and be
    // independent of frame rate
    float currentSpeed = movementSpeed * dt;

    // Check for user input
    Input& input = Input::GetInstance();

    // Speed up or down as necessary
    if (input.KeyDown(VK_SHIFT)) { currentSpeed *= 5; } // Speed up by 5x
    if (input.KeyDown(VK_CONTROL)) { currentSpeed *= 0.1f; } // Slow down by 0.1x

    // Get movement from input
    if (input.KeyDown('W')) { transform.MoveRelative(0, 0, currentSpeed); } // Move forward
    if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -currentSpeed); } // Move backward
    if (input.KeyDown('A')) { transform.MoveRelative(-currentSpeed, 0, 0); } // Move left
    if (input.KeyDown('D')) { transform.MoveRelative(currentSpeed, 0, 0); } // Move right

    // Check if the left mouse button is being held down
    if (input.MouseLeftDown())
    {
        // Determine how far the mouse has moved since last frame
        float xDiff = mouseLookSpeed * input.GetMouseXDelta();
        float yDiff = mouseLookSpeed * input.GetMouseYDelta();

        // Rotate in the opposite order
        transform.SetRotation(yDiff, xDiff, 0);

        // Clamp the X rotation to prevent camera flipping
        XMFLOAT3 currentRot = transform.GetPitchYawRoll();

        // Clamp the upper bound
        if (currentRot.x > XM_PIDIV2)
            currentRot.x = XM_PIDIV2;

        // Clamp the lower bound
        if (currentRot.x < -XM_PIDIV2)
            currentRot.x = -XM_PIDIV2;

        // Set the clamp
        transform.SetRotation(currentRot);
    }

    // Update the view matrix
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    // Get the camera's forward and position vectors
    XMFLOAT3 forward = transform.GetForward();
    XMFLOAT3 position = transform.GetPosition();

    // Create the view matrix
    XMMATRIX view = XMMatrixLookToLH(
        XMLoadFloat3(&position),
        XMLoadFloat3(&forward),
        XMVectorSet(0, 1, 0, 0)
    );

    // Set the view matrix
    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    // Update the aspect ratio
    this->aspectRatio = aspectRatio;

    // Establish a projection matrix
    XMMATRIX projectionMatrix;

    // Determine which type of projection to use
    switch (projectionType)
    {
    case ProjectionType::Perspective: // Perspective Camera
        projectionMatrix = XMMatrixPerspectiveFovLH(
            fieldOfView,    // Field of view angle
            aspectRatio,    // Aspect ratio
            nearClip,       // Near clip plane distance
            farClip         // Far clip plane distance
        );
        break;

    case ProjectionType::Orthographic: // Orthographic Camera
        projectionMatrix = XMMatrixOrthographicLH(
            orthographicWidth,                  // Projection width
            orthographicWidth / aspectRatio,    // Project height
            nearClip,                           // Near clip plane distance
            farClip                             // Far clip plane distance
        );
        break;

    default: // Default to perspective view
        projectionMatrix = XMMatrixPerspectiveFovLH(
            fieldOfView,    // Field of view angle
            aspectRatio,    // Aspect ratio
            nearClip,       // Near clip plane distance
            farClip         // Far clip plane distance
        );
    }

    // Set the projection matrix
    XMStoreFloat4x4(&projMatrix, projectionMatrix);
}
