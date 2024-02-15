#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	position(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	dirtyMatrices(false)
{
	// Initialize matrices
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInvTransMatrix, XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	// Set the position
	position.x = x;
	position.y = y;
	position.z = z;

	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 posiiton)
{
	// Set the position
	this->position = position;

	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	// Set the rotation
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;

	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	// Set the rotation
	this->rotation = rotation;

	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::SetScale(float x, float y, float z)
{
	// Set the scale
	scale.x = x;
	scale.y = y;
	scale.z = z;

	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	// Set the scale
	this->scale = scale;

	// Notify dirty matrices
	dirtyMatrices = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Update the matrices
	UpdateMatrices();

	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	// Update the matrices
	UpdateMatrices();

	return worldInvTransMatrix;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	// Update the vectors
	UpdateVectors();
	
	return rightVec;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	// Update the vectors
	UpdateVectors();

	return upVec;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	// Update the vectors
	UpdateVectors();

	return forwardVec;
}

void Transform::UpdateMatrices()
{
	// If there are no dirty matrices, return
	if (!dirtyMatrices)
		return;

	// Create the transformation matrices
	XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX sc = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

	// Create the world matrix
	XMMATRIX world = sc * rot * trans;

	// Store the matrices
	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInvTransMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

	// Clean the matrices
	dirtyMatrices = false;
}

void Transform::UpdateVectors()
{
	// If there are no dirty vectors, return
	if (!dirtyVectors)
		return;

	// Update all vectors
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMStoreFloat3(&rightVec, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotQuat));
	XMStoreFloat3(&upVec, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotQuat));
	XMStoreFloat3(&forwardVec, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotQuat));

	// Clean the vectors
	dirtyVectors = false;
}

void Transform::MoveRelative(float x, float y, float z)
{
	// Create the movement vector
	XMVECTOR directionVec = XMVectorSet(x, y, z, 0);

	// Represent the transform's rotation using a quaternion
	XMVECTOR currentRotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));

	// Rotate to the given direction
	XMVECTOR directedRot = XMVector3Rotate(directionVec, currentRotQuat);

	// Load the existing position and add it to the new directoin
	XMVECTOR finalPos = XMVectorAdd(XMLoadFloat3(&position), directedRot);

	// Store the new position
	XMStoreFloat3(&position, finalPos);
	
	// Notify dirty matrices
	dirtyMatrices = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	// Call the base function using the components of the XMFLOAT3
	MoveRelative(offset.x, offset.y, offset.z);
}
