#pragma once
#include <memory> // For shared_ptr

#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

class GameEntity
{
private:
	// Variables
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	Transform* GetTransform();

	// Setters
	void SetMaterial(std::shared_ptr<Material> material);

	void Draw();
};

