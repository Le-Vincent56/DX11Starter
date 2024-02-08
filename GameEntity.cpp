#include "GameEntity.h"

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return std::shared_ptr<Mesh>();
}

Transform& GameEntity::GetTransform()
{
	return transform;
}

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	// Initialize variables
	this->mesh = mesh;
}
