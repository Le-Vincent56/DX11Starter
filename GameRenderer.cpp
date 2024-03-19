#include "GameRenderer.h"
#include <algorithm>

#include "PathHelpers.h"

// Include ImGUI
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <iostream>

using namespace DirectX;

// --------------------------------------------------------
// Constructor
// --------------------------------------------------------
GameRenderer::GameRenderer(
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
	Microsoft::WRL::ComPtr<ID3D11Device> _device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV)
	:
	swapChain(_swapChain), device(_device), context(_context), backBufferRTV(_backBufferRTV),
	depthBufferDSV(_depthBufferDSV)
{
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
GameRenderer::~GameRenderer()
{
}

// --------------------------------------------------------
// Get the background color
// --------------------------------------------------------
float* GameRenderer::GetBGColor()
{
	return this->bgColor;
}

std::shared_ptr<SimplePixelShader> GameRenderer::GetPixelShader()
{
	return this->pixelShader;
}

std::shared_ptr<SimpleVertexShader> GameRenderer::GetVertexShader()
{
	return this->vertexShader;
}

std::vector<std::shared_ptr<GameEntity>> GameRenderer::GetRenderedEntities()
{
	return this->renderEntities;
}

std::shared_ptr<LightManager> GameRenderer::GetLightManager()
{
	return this->lightManager;
}

// --------------------------------------------------------
// Handle Renderer intialization
// --------------------------------------------------------
void GameRenderer::Init()
{
	// Load shaders
	LoadShaders();

	// Create light manager
	lightManager = std::make_shared<LightManager>(this->pixelShader);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void GameRenderer::LoadShaders()
{
	// Use Simple Shader to create a pixel and vertex shader
	pixelShader = std::make_shared<SimplePixelShader>(
		device,
		context,
		FixPath(L"CustomPS.cso").c_str()
	);
	vertexShader = std::make_shared<SimpleVertexShader>(
		device,
		context,
		FixPath(L"VertexShader.cso").c_str()
	);
}

// --------------------------------------------------------
// Compare the materials of two entities
// --------------------------------------------------------
bool GameRenderer::CompareEntityMaterials(const std::shared_ptr<GameEntity>& entity1, const std::shared_ptr<GameEntity>& entity2)
{
	// Compare pointer addresses
	return entity1->GetMaterial().get() < entity2->GetMaterial().get();
}

// --------------------------------------------------------
// Sort a list of entities by their material
// --------------------------------------------------------
void GameRenderer::SortByMaterial(std::vector<std::shared_ptr<GameEntity>>& entities)
{
	// Sort the entities using the materials as a lambda function
	std::sort(entities.begin(), entities.end(), CompareEntityMaterials);
}

// --------------------------------------------------------
// Choose what entities to render and store them in a list
// --------------------------------------------------------
void GameRenderer::SelectRenderableEntities(std::vector<std::shared_ptr<GameEntity>>& gameEntities)
{
	// FOR NOW, set the render entities to game entities,
	// LATER, this function will take care of what entities need to be rendered
	renderEntities = gameEntities;
}

// --------------------------------------------------------
// Update the Renderer
// --------------------------------------------------------
void GameRenderer::Update(float& totalTime, std::vector<std::shared_ptr<GameEntity>>& gameEntities)
{
	// Update total time
	this->totalTime = totalTime;

	// Update which entities to render
	SelectRenderableEntities(gameEntities);

	// Sort renderable entities by their material
	SortByMaterial(renderEntities);
}

// --------------------------------------------------------
// Render the game
// --------------------------------------------------------
void GameRenderer::Draw(bool vsync, bool deviceSupportsTearing, BOOL isFullscreen, 
	std::shared_ptr<Camera> camera)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Set pixel shader frame data
	pixelShader->SetFloat3("cameraPos", camera->GetTransform()->GetPosition());
	pixelShader->CopyBufferData("FrameData");

	// Set vertex shader frame data
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());
	vertexShader->CopyBufferData("FrameData");

	// Draw entities
	for (int i = 0; i < renderEntities.size(); ++i)
	{
		// Send light data
		lightManager->SetPixelData();
		
		// Prepare the material's shader data
		renderEntities[i]->GetMaterial()->PrepareMaterial(
			renderEntities[i]->GetTransform(), 
			lightManager->GetAmbientTerm(),
			totalTime
		);

		// Render the entity
		renderEntities[i]->Draw();
	}

	// Render UI
	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}
