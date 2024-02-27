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
    // Update the view matrix
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);

    // Create the view matrix
    XMMATRIX view = XMMatrixLookToLH(
        XMLoadFloat3(&transform.GetPosition()),
        XMLoadFloat3(&transform.GetForward()),
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

    // Set projection matrix using Perspective Projection
    if (projectionType == ProjectionType::Perspective)
    {
        projectionMatrix = XMMatrixPerspectiveFovLH(
            fieldOfView,    // Field of view angle
            aspectRatio,    // Aspect ratio
            nearClip,       // Near clip plane distance
            farClip         // Far clip plane distance
        );
    }
    else // Set projection matrix using Orthographic Projection
    {
        projectionMatrix = XMMatrixOrthographicLH(
            orthographicWidth,                  // Projection width
            orthographicWidth / aspectRatio,    // Project height
            nearClip,                           // Near clip plane distance
            farClip                             // Far clip plane distance
        );
    }

    // Set the projection matrix
    XMStoreFloat4x4(&projMatrix, projectionMatrix);
}
