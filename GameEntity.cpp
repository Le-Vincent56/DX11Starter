#include "GameEntity.h"
#include <DirectXMath.h>

using namespace DirectX;

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return this->mesh;
}

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return this->material;
}

Transform* GameEntity::GetTransform()
{
	return &this->transform;
}

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
	: mesh(mesh), material(material)
{
}

void GameEntity::SetMaterial(std::shared_ptr<Material> material)
{
	this->material = material;
}

void GameEntity::Draw()
{
	// Draw the mesh
	mesh->Draw();
}
