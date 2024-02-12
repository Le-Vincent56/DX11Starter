#pragma once
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory> // Include Memory for shared_ptr
#include <vector>

// Renderer classes
#include "DXCore.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "GameRenderer.h"


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
	void LoadShaders(); 
	void CreateGeometry();
	void CreateEntities();
	void RefreshUI(const float& deltaTime);
	void BuildUI();
	void UpdateEntities(const float& deltaTime, const float& totalTime);

	// Renderer
	GameRenderer* gameRenderer;
	
	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Inspector UI Variables
	int currentTab = 0;
	bool showDemoWindow = true;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Entities
	std::vector<std::shared_ptr<GameEntity>> entities;
};

