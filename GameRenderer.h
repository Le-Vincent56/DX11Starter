#pragma once
#include <vector>

#include "BufferStructs.h"
#include "GameEntity.h"

class GameRenderer
{
private:
	// D3D Objects
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	// Shaders
	VertexShaderData vsData;

	// Entities
	std::vector<std::shared_ptr<GameEntity>> renderEntities;

	// Variables
	float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };

public:
	GameRenderer(
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
		Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV
	);
	~GameRenderer();

	// Getters
	float* GetBGColor();
	VertexShaderData GetVSData();
	
	// Setters
	void SetVSData(VertexShaderData vsData);

	// Initialize Functions
	void Init();
	void InitConstantBuffer();

	// Update Functions
	void SelectRenderableEntities(std::vector<std::shared_ptr<GameEntity>>& gameEntities);
	void UpdateConstBuffers();
	void Update(std::vector<std::shared_ptr<GameEntity>>& gameEntities);

	// Draw Functions
	void Draw(bool vsync, bool deviceSupportsTearing, BOOL isFullscreen);
};

