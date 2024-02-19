#include "GameRenderer.h"

// Include ImGUI
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

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
	// Zero out shader data
	vsData = {};
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
	return bgColor;
}

// --------------------------------------------------------
// Get the current VertexShader data
// --------------------------------------------------------
VertexShaderData GameRenderer::GetVSData()
{
	return vsData;
}

// --------------------------------------------------------
// Set VertexShader data
// --------------------------------------------------------
void GameRenderer::SetVSData(VertexShaderData vsData)
{
	this->vsData = vsData;
}

// --------------------------------------------------------
// Handle Renderer intialization
// --------------------------------------------------------
void GameRenderer::Init()
{
	// Initialize the constant buffer
	InitConstantBuffer();
}

// --------------------------------------------------------
// Initialize constant buffers
// --------------------------------------------------------
void GameRenderer::InitConstantBuffer()
{
	// Get the size of the VertexShaderStruct
	unsigned int byteWidth = sizeof(VertexShaderData);
	byteWidth = (byteWidth + 15) / 16 * 16;

	// Create constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = byteWidth;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	device->CreateBuffer(&cbDesc, 0, constBuffer.GetAddressOf());

	// Set default shader data
	vsData.colorTint = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	vsData.world = XMFLOAT4X4(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);

	// Set constant buffers for shader data
	context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constBuffer.GetAddressOf() // Array of buffers (or address of just one)
	);
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
// Update the constant buffers
// --------------------------------------------------------
void GameRenderer::UpdateConstBuffers()
{
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(constBuffer.Get(), 0);
}

// --------------------------------------------------------
// Update the Renderer
// --------------------------------------------------------
void GameRenderer::Update(std::vector<std::shared_ptr<GameEntity>>& gameEntities)
{
	// Update the constant buffers
	UpdateConstBuffers();

	// Update Entities
	SelectRenderableEntities(gameEntities);
}

// --------------------------------------------------------
// Render the game
// --------------------------------------------------------
void GameRenderer::Draw(bool vsync, bool deviceSupportsTearing, BOOL isFullscreen, )
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

	// Draw entities
	for (int i = 0; i < renderEntities.size(); ++i)
	{
		// Update shader info for each entity
		vsData.world = renderEntities[i]->GetTransform()->GetWorldMatrix();

		// Update constant buffers
		UpdateConstBuffers();

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
