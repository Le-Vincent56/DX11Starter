#pragma once
#include <vector>
#include <unordered_map>

#include "GameEntity.h"
#include "Camera.h"
#include "LightManager.h"
#include "Skybox.h"

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
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> shadowShader;

	// Entities
	std::vector<std::shared_ptr<GameEntity>> renderEntities;

	// Light manager
	std::shared_ptr<LightManager> lightManager;

	// Skybox
	std::shared_ptr<Skybox> skybox;

	// Shadows
	int shadowMapResolution = 1024;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;

	// Post processing
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;
	std::shared_ptr<SimplePixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;

	// Blur
	std::shared_ptr<SimplePixelShader> blurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurSRV;
	int blurRadius = 1;

	// Pixelate
	std::shared_ptr<SimplePixelShader> pixelatePS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pixelateRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pixelateSRV;
	int pixelSize = 5;

	// Variables
	float bgColor[4] = { 0.376f, 0.667f, 0.8f, 1.0f };
	float totalTime = 0;
	int windowWidth;
	int windowHeight;

	// Helper functions
	static bool CompareEntityMaterials(const std::shared_ptr<GameEntity>& entity1, const std::shared_ptr<GameEntity>& entity2);
	void SortByMaterial(std::vector<std::shared_ptr<GameEntity>>& entities);

public:
	GameRenderer(
		int windowWidth, int windowHeight,
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
		Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV
	);
	~GameRenderer();

	// Getters
	float* GetBGColor();
	int GetBlurRadius() const;
	int GetPixelSize() const;
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::vector<std::shared_ptr<GameEntity>> GetRenderedEntities();
	std::shared_ptr<LightManager> GetLightManager();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowSRV();

	// Setters
	void SetBlurRadius(int blurRadius);
	void SetPixelSize(int pixelSize);

	// Initialize Functions
	void Init();
	void LoadShaders();
	void CreateSkybox(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, std::shared_ptr<Mesh> skyMesh);
	void InitShadows();
	void InitPostProcessing();
	void ResizePostProcess(
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv
	);

	// Update Functions
	void SelectRenderableEntities(std::vector<std::shared_ptr<GameEntity>>& gameEntities);
	void Update(float& totalTime, std::vector<std::shared_ptr<GameEntity>>& gameEntities);
	void RenderShadows();
	void RenderPostProcessing();
	void Blur();
	void Pixelate();

	// Draw Functions
	void Draw(bool vsync, bool deviceSupportsTearing, BOOL isFullscreen, 
		std::shared_ptr<Camera> camera);
};

