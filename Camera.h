#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include "Input.h"

enum class ProjectionType
{
	Perspective,
	Orthographic
};

class Camera
{
private:
	Transform* transform;

	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	// Control fields
	float movementSpeed;
	float mouseLookSpeed;

	// Customization fields
	float fieldOfView;
	float aspectRatio;
	float nearClip;
	float farClip;
	float orthographicWidth;
	ProjectionType projectionType = ProjectionType::Perspective;

public:
	Camera(
		DirectX::XMFLOAT3 position,
		float movementSpeed,
		float mouseLookSpeed,
		float fieldOfView,
		float aspectRatio,
		float nearClip = 0.01f,
		float farClip = 100.0f,
		ProjectionType projectionType = ProjectionType::Perspective
	);

	Camera(
		float x,
		float y,
		float z,
		float movementSpeed,
		float mouseLookSpeed,
		float fieldOfView,
		float aspectRatio,
		float nearClip = 0.01f,
		float farClip = 100.0f,
		ProjectionType projectionType = ProjectionType::Perspective
	);

	~Camera();

	// Setters
	void SetMovementSpeed(float movementSpeed);
	void SetMouseLookSpeed(float mouseLookSpeed);
	void SetFieldOfView(float fieldOfView);
	void SetNearClip(float nearClip);
	void SetFarClip(float farClip);
	void SetOrthographicWidth(float orthographicWidth);
	void SetProjectionType(ProjectionType projectionType);

	// Getters
	Transform* GetTransform();
	DirectX::XMFLOAT4X4 GetView() const;
	DirectX::XMFLOAT4X4 GetProjection() const;
	float GetMovementSpeed() const;
	float GetMouseLookSpeed() const;
	float GetFieldOfView() const;
	float GetAspectRatio() const;
	float GetNearClip() const;
	float GetFarClip() const;
	float GetOrthographicWidth() const;
	ProjectionType GetProjectionType() const;

	// Update Functions
	void Update();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
};

