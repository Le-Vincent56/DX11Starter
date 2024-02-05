#pragma once

#include "DXCore.h"
#include "Mesh.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory> // Include Memory for shared_ptr
#include <vector>
#include "BufferStructs.h"

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
	void InitConstBuffer();
	void RefreshUI(const float& deltaTime);
	void BuildUI();
	void UpdateConstBuffers();

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;
	
	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Shader data
	VertexShaderData vsData;

	// Inspector UI Variables
	int currentTab = 0;
	float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool showDemoWindow = true;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;
};

