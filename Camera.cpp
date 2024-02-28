#include "Camera.h"
#include <iostream>

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
    projectionType(projectionType),
    orthographicWidth(2.0f)
{
    // Set the transform position
    this->transform = new Transform();
    transform->SetPosition(position);

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
    :
    movementSpeed(movementSpeed),
    mouseLookSpeed(mouseLookSpeed),
    fieldOfView(fieldOfView),
    aspectRatio(aspectRatio),
    nearClip(nearClip),
    farClip(farClip),
    projectionType(projectionType),
    orthographicWidth(2.0f)
{
    // Set the transform position
    this->transform = new Transform();
    transform->SetPosition(x, y, z);

    // Update camera matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
    delete transform;
    transform = nullptr;
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
    return this->transform;
}

DirectX::XMFLOAT4X4 Camera::GetView() const
{
    return this->viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection() const
{
    return this->projMatrix;
}

float Camera::GetMovementSpeed() const
{
    return this->movementSpeed;
}

float Camera::GetMouseLookSpeed() const
{
    return this->mouseLookSpeed;
}

float Camera::GetFieldOfView() const
{
    return this->fieldOfView;
}

float Camera::GetAspectRatio() const
{
    return this->aspectRatio;
}

float Camera::GetNearClip() const
{
    return this->nearClip;
}

float Camera::GetFarClip() const
{
    return this->farClip;
}

float Camera::GetOrthographicWidth() const
{
    return this->orthographicWidth;
}

ProjectionType Camera::GetProjectionType() const
{
    return this->projectionType;
}

void Camera::Update()
{
    // Update the view matrix
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    // Get the position and forward vectors, and the world up vector
    XMFLOAT3 position = transform->GetPosition();
    XMVECTOR positionVec = XMLoadFloat3(&position);
    XMFLOAT3 forward = transform->GetForward();
    XMVECTOR forwardVec = XMLoadFloat3(&forward);
    XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);

    // Create the view matrix
    XMMATRIX view = XMMatrixLookToLH(
        positionVec,
        forwardVec,
        worldUp
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

    switch (projectionType)
    {
    case ProjectionType::Perspective: // Perspective D
        projectionMatrix = XMMatrixPerspectiveFovLH(
            fieldOfView,    // Field of view angle
            aspectRatio,    // Aspect ratio
            nearClip,       // Near clip plane distance
            farClip         // Far clip plane distance
        );
        break;

    case ProjectionType::Orthographic: // Orthographic Projection
        projectionMatrix = XMMatrixOrthographicLH(
            orthographicWidth,                  // Projection width
            orthographicWidth / aspectRatio,    // Project height
            nearClip,                           // Near clip plane distance
            farClip                             // Far clip plane distance
        );
        break;

    default: // Default to Perspective projection
        projectionMatrix = XMMatrixPerspectiveFovLH(
            fieldOfView,    // Field of view angle
            aspectRatio,    // Aspect ratio
            nearClip,       // Near clip plane distance
            farClip         // Far clip plane distance
        );
        break;
    }

    // Set the projection matrix
    XMStoreFloat4x4(&projMatrix, projectionMatrix);
}
