#pragma once
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory> // Include Memory for shared_ptr
#include <vector>

// Renderer classes
#include "DXCore.h"
#include "Mesh.h"
#include "GameRenderer.h"
#include "Camera.h"
#include "UserInput.h"
#include "SimpleShader.h"
#include "Material.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();
	void CreateMaterials();
	void CreateEntities();
	void RefreshUI(const float& deltaTime);
	void BuildUI();
	void UpdateEntities(const float& deltaTime, const float& totalTime);

	// UI Methods
	void ConstructGeneralUI();
	void ConstructInputUI();
	void ConstructMeshesUI();
	void ConstructShadersUI();
	void ConstructEntitiesUI();
	void ConstructCameraUI();
	void ConstructLightUI();

	// Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCamera;

	// Renderer
	std::shared_ptr<GameRenderer> gameRenderer;

	// Inspector UI Variables
	int currentTab = 0;
	bool showDemoWindow = true;

	// Materials
	std::vector<std::shared_ptr<Material>> materials;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Entities
	std::vector<std::shared_ptr<GameEntity>> entities;

	// User input
	std::shared_ptr<UserInput> userInput;
};

