#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"

#include <memory> // For shared_ptr

class GameEntity
{
private:
	// Variables
	Transform transform;
	std::shared_ptr<Mesh> mesh;

public:
	GameEntity(std::shared_ptr<Mesh> mesh);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw();
};

