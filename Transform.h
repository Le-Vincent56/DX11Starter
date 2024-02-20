#pragma once
#include <DirectXMath.h>

class Transform
{
private:
	// Transform variables
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	// Matrix variables
	bool dirtyMatrices;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInvTransMatrix;

	// Vector variables
	bool dirtyVectors;
	DirectX::XMFLOAT3 rightVec;
	DirectX::XMFLOAT3 upVec;
	DirectX::XMFLOAT3 forwardVec;

public:
	// Constructor/Destructor
	Transform();
	~Transform();

	//
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 pyrRotation);

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 posiiton);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 rotation);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();

	// Update Functions
	void UpdateMatrices();
	void UpdateVectors();
	void MoveRelative(float x, float y, float z);
	void MoveRelative(DirectX::XMFLOAT3 offset);
};