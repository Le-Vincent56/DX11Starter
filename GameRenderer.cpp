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
	int windowWidth, int windowHeight,
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
	Microsoft::WRL::ComPtr<ID3D11Device> _device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV)
	:
	windowWidth(windowWidth), windowHeight(windowHeight),
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

int GameRenderer::GetBlurRadius() const
{
	return this->blurRadius;
}

int GameRenderer::GetPixelSize() const
{
	return this->pixelSize;
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

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GameRenderer::GetShadowSRV()
{
	return this->shadowSRV;
}

void GameRenderer::SetBlurRadius(int blurRadius)
{
	this->blurRadius = blurRadius;
}

void GameRenderer::SetPixelSize(int pixelSize)
{
	this->pixelSize = pixelSize;
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

	// Initialize shadows
	InitShadows();

	// Initialize post process
	InitPostProcessing();
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

	shadowShader = std::make_shared<SimpleVertexShader>(
		device,
		context,
		FixPath(L"ShadowVertexShader.cso").c_str()
	);

	ppVS = std::make_shared<SimpleVertexShader>(
		device,
		context,
		FixPath(L"FullscreenVS.cso").c_str()
	);

	ppPS = std::make_shared<SimplePixelShader>(
		device,
		context,
		FixPath(L"PostProcessPixelShader.cso").c_str()
	);

	blurPS = std::make_shared<SimplePixelShader>(
		device,
		context,
		FixPath(L"BlurPixelShader.cso").c_str()
	);

	pixelatePS = std::make_shared<SimplePixelShader>(
		device,
		context,
		FixPath(L"PixelatePixelShader.cso").c_str()
	);
}

void GameRenderer::CreateSkybox(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, std::shared_ptr<Mesh> skyMesh)
{
	skybox = std::make_shared<Skybox>(
		FixPath(L"../../Textures/Skybox/right.png").c_str(),
		FixPath(L"../../Textures/Skybox/left.png").c_str(),
		FixPath(L"../../Textures/Skybox/up.png").c_str(),
		FixPath(L"../../Textures/Skybox/down.png").c_str(),
		FixPath(L"../../Textures/Skybox/front.png").c_str(),
		FixPath(L"../../Textures/Skybox/back.png").c_str(),
		this->device,
		this->context,
		sampler,
		skyMesh
	);
}

void GameRenderer::InitShadows()
{
	// Create shadow map texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution;
	shadowDesc.Height = shadowMapResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create shadow depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf()
	);

	// Create shadow SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf()
	);

	// Create shadow rasterizer
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create shadow sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create light view maxtix
	XMVECTOR lightDirection = XMVectorSet(1.0f, -1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookToLH(
		-lightDirection * 3,
		lightDirection,
		XMVectorSet(0, 1, 0, 0)
	);
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	// Create light projection matrix
	float lightProjectionSize = 35.0f;
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f
	);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

void GameRenderer::InitPostProcessing()
{
	// Create sampler state
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	ResizePostProcess(ppRTV, ppSRV);
	ResizePostProcess(blurRTV, blurSRV);
	ResizePostProcess(pixelateRTV, pixelateSRV);
}

void GameRenderer::ResizePostProcess(
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv, 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	// Reset the resources
	rtv.Reset();
	srv.Reset();

	// Create the texture for the render target
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = this->windowWidth;
	textureDesc.Height = this->windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> postProcessTexture;
	device->CreateTexture2D(&textureDesc, 0, postProcessTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(
		postProcessTexture.Get(),
		&rtvDesc,
		rtv.ReleaseAndGetAddressOf()
	);

	// Create the Shader Resource View
	device->CreateShaderResourceView(
		postProcessTexture.Get(),
		0,
		srv.ReleaseAndGetAddressOf()
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

void GameRenderer::RenderShadows()
{
	// Set shadow rasterizer state
	context->RSSetState(shadowRasterizer.Get());

	// Clear the shader buffer
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set the output merger state
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Deactivate pixel shader
	context->PSSetShader(0, 0, 0);

	// Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set shaders
	shadowShader->SetShader();
	shadowShader->SetMatrix4x4("view", lightViewMatrix);
	shadowShader->SetMatrix4x4("projection", lightProjectionMatrix);

	// Draw all entities
	for (auto& e : renderEntities)
	{
		// Set buffer data
		shadowShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowShader->CopyAllBufferData();

		// Draw meshes directly
		e->GetMesh()->Draw();
	}

	// Reset pipeline
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
	context->OMSetRenderTargets(
		1,
		backBufferRTV.GetAddressOf(),
		depthBufferDSV.Get()
	);
}

void GameRenderer::RenderPostProcessing()
{
	// Activate  the vertex shader
	ppVS->SetShader();
	
	Blur();
	Pixelate();
}

void GameRenderer::Blur()
{
	// Set the render target view
	context->OMSetRenderTargets(1, blurRTV.GetAddressOf(), 0);

	// Set the shader and bind resources
	blurPS->SetShader();
	blurPS->SetShaderResourceView("Pixels", ppSRV.Get());
	blurPS->SetSamplerState("ClampSampler", ppSampler.Get());

	// Set cbuffer data
	blurPS->SetInt("blurRadius", this->blurRadius);
	blurPS->SetFloat("pixelWidth", 1.0f/(float)this->windowWidth);
	blurPS->SetFloat("pixelHeight", 1.0f/(float)this->windowHeight);
	blurPS->CopyAllBufferData();

	context->Draw(3, 0);
}

void GameRenderer::Pixelate()
{
	// Restore the back buffer
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Set the shader and bind resources
	pixelatePS->SetShader();
	pixelatePS->SetShaderResourceView("Pixels", blurSRV.Get());
	pixelatePS->SetSamplerState("ClampSampler", ppSampler.Get());

	// Set cbuffer data
	pixelatePS->SetFloat("pixelSize", (float)this->pixelSize);
	pixelatePS->SetFloat2("textureSize", DirectX::XMFLOAT2((float)this->windowWidth, (float)this->windowHeight));
	pixelatePS->CopyAllBufferData();

	context->Draw(3, 0);
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

		// Clear the post-processing render targets
		const float clearColor[4] = {
			1.0f,
			1.0f,
			1.0f,
			1.0f
		};
		context->ClearRenderTargetView(ppRTV.Get(), clearColor);
		context->ClearRenderTargetView(blurRTV.Get(), clearColor);
		context->ClearRenderTargetView(pixelateRTV.Get(), clearColor);

		// Render shadows
		RenderShadows();

		// Swap the active render target
		context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthBufferDSV.Get());
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

		// Set shadow data
		vertexShader->SetMatrix4x4("lightView", lightViewMatrix);
		vertexShader->SetMatrix4x4("lightProjection", lightProjectionMatrix);
		pixelShader->SetShaderResourceView("ShadowMap", shadowSRV);
		pixelShader->SetSamplerState("ShadowSampler", shadowSampler);
		
		// Prepare the material's shader data
		renderEntities[i]->GetMaterial()->PrepareMaterial(
			renderEntities[i]->GetTransform(), 
			lightManager->GetAmbientTerm(),
			totalTime
		);

		// Render the entity
		renderEntities[i]->Draw();
	}

	// Draw the skybox last
	skybox->Draw(camera);

	// Render post processing
	RenderPostProcessing();

	// Unbind the shadow map
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, 128, nullSRVs);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Render UI
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

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
