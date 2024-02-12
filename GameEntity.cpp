#include "GameEntity.h"
#include <DirectXMath.h>

using namespace DirectX;

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	// Initialize variables
	this->mesh = mesh;
}

void GameEntity::Draw()
{
	// Draw the mesh
	mesh->Draw();
}
