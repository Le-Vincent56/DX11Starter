#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"

// Include ImGUI
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include <math.h>

#include <WICTextureLoader.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	activeCamera = 0;
	gameRenderer = nullptr;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Create a renderer
	gameRenderer = std::make_shared<GameRenderer>(this->swapChain,
		this->device, this->context, this->backBufferRTV, this->depthBufferDSV);

	// Initialize the renderer - initializes shaders as well
	gameRenderer->Init();

	// Create geometry
	CreateGeometry();

	// Create assets
	CreateAssets();
	
	// Create Entities
	CreateEntities();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Create a camera
	cameras.push_back(
		std::make_shared<Camera>(
			-5.0f, 2.0f, -10.0f,				// Position
			5.0f,								// Movement speed
			0.002f,								// Look speed
			XM_PIDIV4,							// Field of view
			(float)windowWidth / windowHeight,	// Aspect Ratio
			0.01f,								// Near clip
			100.0f,								// Far clip
			ProjectionType::Perspective			// Projection type
		)
	);

	cameras.push_back(
		std::make_shared<Camera>(
			XMFLOAT3(5.0f, 2.0f, -10.0f),		// Using alternate position constructor
			5.0f,
			0.002f,
			XM_PIDIV2,
			(float)windowWidth / windowHeight,
			0.01f,
			100.0f,
			ProjectionType::Perspective
		)
	);

	cameras.push_back(
		std::make_shared<Camera>(
			0.0f, 0.0f, -5.0f,
			5.0f,
			0.002f,
			XM_PI,
			(float)windowWidth / windowHeight,
			0.01f,
			100.0f,
			ProjectionType::Orthographic		// Orthographic projection
		)
	);

	// Set the default camera to the first one
	activeCamera = 0;

	// Create a user input controller
	userInput = std::make_shared<UserInput>(*cameras[activeCamera]->GetTransform(), ControlType::Camera);
	userInput->SetMovementSpeed(cameras[activeCamera]->GetMovementSpeed());
	userInput->SetLookSpeed(cameras[activeCamera]->GetMouseLookSpeed());

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();
}

void Game::CreateAssets()
{
	// Create sampler
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC samplerDesc = {};

	// Define how to handle UV's outside of the 0-1 range
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	// Set a filter
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8; // Range between 1-16, higher is better but slower

	// Enable mipmapping
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Store the sampler description into the sampler
	device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	// Create materials
	CreateMaterials(sampler);

	// Create skybox
	gameRenderer->CreateSkybox(sampler, meshes[2]);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	meshes.push_back(
		std::make_shared<Mesh>(
			context,
			swapChain,
			device,
			FixPath("../../Assets/sphere.obj").c_str()
		)
	);

	meshes.push_back(
		std::make_shared<Mesh>(
			context,
			swapChain,
			device,
			FixPath("../../Assets/helix.obj").c_str()
		)
	);

	meshes.push_back(
		std::make_shared<Mesh>(
			context,
			swapChain,
			device,
			FixPath("../../Assets/cube.obj").c_str()
		)
	);
}

// --------------------------------------------------------
// Create materials
// --------------------------------------------------------
void Game::CreateMaterials(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	// Create ShaderResourceViews
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> marbleAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> marbleNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> marbleRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> marbleMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roofingTileAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roofingTileNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roofingTileRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roofingTileMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> steelAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> steelNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> steelRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> steelMetalnessSRV;

	// Load Marble
	{
		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Marble_Albedo.png").c_str(),
			0,
			marbleAlbedoSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Marble_Normal.png").c_str(),
			0,
			marbleNormalSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Marble_Roughness.png").c_str(),
			0,
			marbleRoughnessSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Marble_Metal.png").c_str(),
			0,
			marbleMetalnessSRV.GetAddressOf()
		);
	}

	// Load Roofing Tile
	{
		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/RoofingTile_Albedo.png").c_str(),
			0,
			roofingTileAlbedoSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/RoofingTile_Normal.png").c_str(),
			0,
			roofingTileNormalSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/RoofingTile_Roughness.png").c_str(),
			0,
			roofingTileRoughnessSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/RoofingTile_Metal.png").c_str(),
			0,
			roofingTileMetalnessSRV.GetAddressOf()
		);
	}

	// Load Scratched
	{
		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Scratched_Albedo.png").c_str(),
			0,
			scratchedAlbedoSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Scratched_Normal.png").c_str(),
			0,
			scratchedNormalSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Scratched_Roughness.png").c_str(),
			0,
			scratchedRoughnessSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Scratched_Metal.png").c_str(),
			0,
			scratchedMetalnessSRV.GetAddressOf()
		);
	}

	// Load Steel
	{
		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Steel_Albedo.png").c_str(),
			0,
			steelAlbedoSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Steel_Normal.png").c_str(),
			0,
			steelNormalSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Steel_Roughness.png").c_str(),
			0,
			steelRoughnessSRV.GetAddressOf()
		);

		CreateWICTextureFromFile(
			device.Get(),
			context.Get(),
			FixPath(L"../../Textures/Steel_Metal.png").c_str(),
			0,
			steelMetalnessSRV.GetAddressOf()
		);
	}

	// Create marble texture
	std::shared_ptr<Material> marble = std::make_shared<Material>(XMFLOAT3(1, 1, 1), 0.0f, 0.2f, 1.0f, gameRenderer->GetPixelShader(), gameRenderer->GetVertexShader());
	marble->AddTextureSRV("Albedo", marbleAlbedoSRV);
	marble->AddTextureSRV("NormalMap", marbleNormalSRV);
	marble->AddTextureSRV("RoughnessMap", marbleRoughnessSRV);
	marble->AddTextureSRV("MetalnessMap", marbleMetalnessSRV);
	marble->AddSamplerState("BasicSampler", sampler);
	materials.insert({ "Marble", marble });

	// Create roofing tile texture
	std::shared_ptr<Material> roofingTile = std::make_shared<Material>(XMFLOAT3(1, 1, 1), 0.0f, 0.2f, 1.0f, gameRenderer->GetPixelShader(), gameRenderer->GetVertexShader());
	roofingTile->AddTextureSRV("Albedo", roofingTileAlbedoSRV);
	roofingTile->AddTextureSRV("NormalMap", roofingTileNormalSRV);
	roofingTile->AddTextureSRV("RoughnessMap", roofingTileRoughnessSRV);
	roofingTile->AddTextureSRV("MetalnessMap", roofingTileMetalnessSRV);
	roofingTile->AddSamplerState("BasicSampler", sampler);
	materials.insert({ "Roofing Tile", roofingTile });

	// Create scratched texture
	std::shared_ptr<Material> scratched = std::make_shared<Material>(XMFLOAT3(1, 1, 1), 0.0f, 0.0f, 0.0f, gameRenderer->GetPixelShader(), gameRenderer->GetVertexShader());
	scratched->AddTextureSRV("Albedo", scratchedAlbedoSRV);
	scratched->AddTextureSRV("NormalMap", scratchedNormalSRV);
	scratched->AddTextureSRV("RoughnessMap", scratchedRoughnessSRV);
	scratched->AddTextureSRV("MetalnessMap", scratchedMetalnessSRV);
	scratched->AddSamplerState("BasicSampler", sampler);
	materials.insert({ "Scratched", scratched });

	// Create steel texture
	std::shared_ptr<Material> steel = std::make_shared<Material>(XMFLOAT3(1, 1, 1), 0.0f, 0.0f, 2.0f, gameRenderer->GetPixelShader(), gameRenderer->GetVertexShader());
	steel->AddTextureSRV("Albedo", steelAlbedoSRV);
	steel->AddTextureSRV("NormalMap", steelNormalSRV);
	steel->AddTextureSRV("RoughnessMap", steelRoughnessSRV);
	steel->AddTextureSRV("MetalnessMap", steelMetalnessSRV);
	steel->AddSamplerState("BasicSampler", sampler);
	materials.insert({ "Steel", steel });
}

// --------------------------------------------------------
// Create the game entities
// --------------------------------------------------------
void Game::CreateEntities()
{
	// Create entities and add them to the list
	entities.push_back(
		std::make_shared<GameEntity>(
			meshes[0],
			materials["Steel"]
		)
	);
	entities[0]->GetTransform()->SetPosition(-10.0f, 0.0f, 0.0f);

	entities.push_back(
		std::make_shared<GameEntity>(
			meshes[0],
			materials["Scratched"]
		)
	);
	entities[1]->GetTransform()->SetPosition(-5.0f, 0.0f, 0.0f);

	entities.push_back(
		std::make_shared<GameEntity>(
			meshes[1],
			materials["Marble"]
		)
	);
	entities[2]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

	entities.push_back(
		std::make_shared<GameEntity>(
			meshes[2],
			materials["Roofing Tile"]
		)
	);
	entities[3]->GetTransform()->SetPosition(5.0f, 0.0f, 0.0f);

	entities.push_back(
		std::make_shared<GameEntity>(
			meshes[2],
			materials["Steel"]
		)
	);
	entities[4]->GetTransform()->SetPosition(10.0f, 0.0f, 0.0f);
}

// --------------------------------------------------------
// Initialize ImGUI for the next frame
// --------------------------------------------------------
void Game::RefreshUI(const float& deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	if(showDemoWindow)
		ImGui::ShowDemoWindow();
}

// --------------------------------------------------------
// Use ImGUI to build the UI
// --------------------------------------------------------
void Game::BuildUI()
{
	// Create a new window called "Inspector"
	ImGui::Begin("Inspector");

	// Build tabs
	{
		ImGui::SameLine();
		if (ImGui::Button("General"))
			currentTab = 0;

		ImGui::SameLine();
		if (ImGui::Button("Input"))
			currentTab = 1;

		ImGui::SameLine();
		if (ImGui::Button("Meshes"))
			currentTab = 2;

		ImGui::SameLine();
		if (ImGui::Button("Shader Data"))
			currentTab = 3;

		ImGui::SameLine();
		if (ImGui::Button("Scene Entities"))
			currentTab = 4;

		ImGui::SameLine();
		if (ImGui::Button("Camera"))
			currentTab = 5;

		ImGui::SameLine();
		if (ImGui::Button("Lights"))
			currentTab = 6;

		ImGui::SameLine();
		if (ImGui::Button("Materials"))
			currentTab = 7;
	}

	// Create a small separator
	ImGui::NewLine();

	switch (currentTab)
	{
	// General Tab
	case 0:
		ConstructGeneralUI();
		break;

	// Input tab
	case 1:
		ConstructInputUI();
		break;

	// Meshes tab
	case 2:
		ConstructMeshesUI();
		break;

	// Shader Data tab
	case 3:
		ConstructShadersUI();
		break;

	// Scene Entities tab
	case 4:
		ConstructEntitiesUI();
		break;

	case 5:
		ConstructCameraUI();
		break;

	case 6:
		ConstructLightUI();
		break;

	case 7:
		ConstructMaterialsUI();
		break;
	}

	// End the "Inspector" window
	ImGui::End();
}

// --------------------------------------------------------
// Update the game entities
// --------------------------------------------------------
void Game::UpdateEntities(const float& deltaTime, const float& totalTime)
{
	// Scale the first and last entity
	float scale = (float)sin(totalTime) * 0.5f + 1.0f;

	// Rotate and scalle all entities
	for (int i = 0; i < entities.size(); ++i)
	{
		entities[i]->GetTransform()->SetRotation(totalTime, 0, totalTime);
		entities[i]->GetTransform()->SetScale(scale, scale, scale);
	}
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the all cameras' projections to match the new aspect ratio
	for (int i = 0; i < cameras.size(); ++i)
	{
		cameras[i]->UpdateProjectionMatrix((float)windowWidth / windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Initialize the UI for the next frame
	RefreshUI(deltaTime);

	// Build the UI
	BuildUI();

	// Update the user input controller
	userInput->Update(deltaTime);

	// Update entities
	//UpdateEntities(deltaTime, totalTime);

	// Update renderer
	gameRenderer->Update(totalTime, entities);

	// Update camera
	cameras[activeCamera]->Update();

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Use the game renderer to draw
	gameRenderer->Draw(vsync, deviceSupportsTearing, isFullscreen, cameras[activeCamera]);
}

void Game::ConstructGeneralUI()
{
	// Display framerate, deltaTime and window resolution
	ImGui::Text("Current Framerate: %f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Current DeltaTime: %f", ImGui::GetIO().DeltaTime);
	ImGui::Text("Window Resolution: %dx%d", windowWidth, windowHeight);

	// Edit the background color
	ImGui::ColorEdit4("Background Color", &gameRenderer->GetBGColor()[0]);

	// Toggle the demo window
	if (ImGui::Button("Toggle ImGUI Demo Window"))
		showDemoWindow = !showDemoWindow;
}

// --------------------------------------------------------
// Construct the Input ImGUI Tab
// --------------------------------------------------------
void Game::ConstructInputUI()
{
	// Display mouse info
	ImGui::Text("Current Mouse Position: (%d, %d)", Input::GetInstance().GetMouseX(), Input::GetInstance().GetMouseY());
	ImGui::Text("Left Mouse Down: %d", Input::GetInstance().MouseLeftDown());
	ImGui::Text("Middle Mouse Down: %d", Input::GetInstance().MouseMiddleDown());
	ImGui::Text("Right Mouse Down: %d", Input::GetInstance().MouseRightDown());
}

// --------------------------------------------------------
// Construct the Meshes ImGUI Tab
// --------------------------------------------------------
void Game::ConstructMeshesUI()
{
	for (int i = 0; i < meshes.size(); i++)
	{
		// Push the current ID
		ImGui::PushID(i);

		// Create the header
		std::string header = "Mesh " + std::to_string(i);
		const char* cHeader = header.c_str();

		// List meshes under header
		if (ImGui::CollapsingHeader(cHeader))
		{
			// Calculate triangles
			int triangleNum = 1;
			if (meshes[i]->GetIndexCount() % 3 == 0)
			{
				triangleNum = meshes[i]->GetIndexCount() / 3;
			}

			// Display mesh number and number of triangles
			ImGui::Text("%d vertices, %d triangles", i, meshes[i]->GetVertexCount(), triangleNum);

			// Display vertices
			for (int v = 0; v < meshes[i]->GetVertices().size(); v++)
			{
				ImGui::Text("\tVertex %d: (%.3f, %.3f, %.3f)",
					v,
					meshes[i]->GetVertices()[v].Position.x,
					meshes[i]->GetVertices()[v].Position.y,
					meshes[i]->GetVertices()[v].Position.z
				);
			}

			// Display indices
			ImGui::Text("Indices (%d): {", meshes[i]->GetIndexCount());
			for (unsigned int ind = 0; ind < meshes[i]->GetIndexCount(); ind++)
			{
				// Put it on the same line and display the index
				ImGui::SameLine();
				if (ind != meshes[i]->GetIndexCount() - 1)
					ImGui::Text("%d,", meshes[i]->GetIndices()[ind]);
				else
					ImGui::Text("%d", meshes[i]->GetIndices()[ind]);
			}
			ImGui::SameLine();
			ImGui::Text("}");
		}

		// Push the current ID
		ImGui::PopID();
	}
}

// --------------------------------------------------------
// Construct the Shaders ImGUI Tab
// --------------------------------------------------------
void Game::ConstructShadersUI()
{
	std::vector<std::shared_ptr<GameEntity>> renderEntities = gameRenderer->GetRenderedEntities();

	for (int i = 0; i < renderEntities.size(); ++i)
	{
		// Push the current ID
		ImGui::PushID(i);

		// Get shader pointers
		std::shared_ptr<SimplePixelShader> ps = renderEntities[i]->GetMaterial()->GetPixelShader();
		std::shared_ptr<SimpleVertexShader> vs = renderEntities[i]->GetMaterial()->GetVertexShader();

		// Get editable variables
		XMFLOAT3 colorTint = renderEntities[i]->GetMaterial()->GetColorTint();
		XMFLOAT4X4 worldMatrix = renderEntities[i]->GetTransform()->GetWorldMatrix();

		// Create the header
		std::string header = "Shader - Entity " + std::to_string(i);
		const char* cHeader = header.c_str();

		// List shaders under headers
		if (ImGui::CollapsingHeader(cHeader))
		{
			// World Matrix display
			ImGui::Text("World Matrix");
			ImGui::InputFloat4("", &worldMatrix._11);
			ImGui::InputFloat4("", &worldMatrix._21);
			ImGui::InputFloat4("", &worldMatrix._31);
			ImGui::InputFloat4("", &worldMatrix._41);
			ImGui::NewLine();

			// Set color tint
			ImGui::Text("Shader Color Tint");
			ImGui::ColorEdit4("Color Tint", &colorTint.x);

			// Color reset button
			if (ImGui::Button("Reset Color Tint"))
				colorTint = XMFLOAT3(1.0f, 1.0f, 1.0f);
		}

		// Set data
		ps->SetFloat3("colorTint", colorTint);
		vs->SetMatrix4x4("world", worldMatrix);

		// Update constant buffers
		ps->CopyAllBufferData();
		vs->CopyAllBufferData();

		// Pop the current ID
		ImGui::PopID();
	}
}

// --------------------------------------------------------
// Construct the Entities ImGUI Tab
// --------------------------------------------------------
void Game::ConstructEntitiesUI()
{
	// Loop through the entities
	for (int i = 0; i < entities.size(); ++i)
	{
		// Push the current ID
		ImGui::PushID(i);

		// Get material data
		XMFLOAT3 materialTint = entities[i]->GetMaterial()->GetColorTint();

		// Get position data
		XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
		XMFLOAT3 pyrRotation = entities[i]->GetTransform()->GetPitchYawRoll();
		XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();
		unsigned int meshCount = entities[i]->GetMesh()->GetIndexCount();

		// Create the header
		std::string header = "Entity " + std::to_string(i + 1);
		const char* cHeader = header.c_str();

		// List entities under headers
		if (ImGui::CollapsingHeader(cHeader))
		{
			ImGui::ColorEdit4("Material Tint", &materialTint.x);
			ImGui::DragFloat3("Position", &position.x);
			ImGui::DragFloat3("Rotation", &pyrRotation.x);
			ImGui::DragFloat3("Scale", &scale.x);
			ImGui::Text("Mesh Index Count: %d", meshCount);
		}

		// Set material data
		entities[i]->GetMaterial()->SetColorTint(materialTint);

		// Set transform data
		entities[i]->GetTransform()->SetPosition(position);
		entities[i]->GetTransform()->SetRotation(pyrRotation);
		entities[i]->GetTransform()->SetScale(scale);

		// Pop the current ID
		ImGui::PopID();
	}
}

// --------------------------------------------------------
// Construct the Camera ImGUI Tab
// --------------------------------------------------------
void Game::ConstructCameraUI()
{
	// Add buttons for every camera within the cameras vector
	ImGui::Text("Select Camera");
	for (int i = 0; i < cameras.size(); ++i)
	{
		// Put all the buttons on the same line
		if (i != 0)
			ImGui::SameLine();

		// Create the button label
		std::string label = "Camera " + std::to_string(i + 1);
		const char* cLabel = label.c_str();

		// Depending on the button pressed, set the active camera
		if (ImGui::Button(cLabel))
			activeCamera = i;
	}

	// Add some spacing
	ImGui::NewLine();
	ImGui::Text("Current Camera Variables");

	// Get the transform variables
	XMFLOAT3 position = cameras[activeCamera]->GetTransform()->GetPosition();
	XMFLOAT3 rotation = cameras[activeCamera]->GetTransform()->GetPitchYawRoll();

	// Get the near and far clip planes
	float nearClip = cameras[activeCamera]->GetNearClip();
	float farClip = cameras[activeCamera]->GetFarClip();

	// Have buttons trigger perspective or orthographic perspectives
	if (ImGui::Button("Perspective"))
		cameras[activeCamera]->SetProjectionType(ProjectionType::Perspective);
	ImGui::SameLine();
	if (ImGui::Button("Orthographic"))
		cameras[activeCamera]->SetProjectionType(ProjectionType::Orthographic);
	
	// Allow the setting of the basic transform variables
	if (ImGui::DragFloat3("Camera Position", &position.x, 0.01f))
		cameras[activeCamera]->GetTransform()->SetPosition(position);
	if (ImGui::DragFloat3("Camera Rotation (Radians, PYR)", &rotation.x, 0.01f))
		cameras[activeCamera]->GetTransform()->SetRotation(rotation);

	// Allow the setting of the near and far clip variables
	if (ImGui::SliderFloat("Near Clip Distance", &nearClip, 0.001f, 1.0f))
		cameras[activeCamera]->SetNearClip(nearClip);
	if (ImGui::SliderFloat("Far Clip Distance", &farClip, 10.0f, 1000.0f))
		cameras[activeCamera]->SetFarClip(farClip);

	// Get the field of view and convert it to degrees
	float fieldOfView = cameras[activeCamera]->GetFieldOfView() * 180.0f / XM_PI;

	// Get the orthographic width
	float orthoWidth = cameras[activeCamera]->GetOrthographicWidth();

	// Change UI based on projection type
	switch (cameras[activeCamera]->GetProjectionType())
	{
	case ProjectionType::Perspective:
		// Allow the setting of the field of view
		if (ImGui::SliderFloat("Field of View (Degrees)", &fieldOfView, 0.01f, 180.0f))
			cameras[activeCamera]->SetFieldOfView(fieldOfView * XM_PI / 180.0f); // Need to convert back to radians
		break;

	case ProjectionType::Orthographic:
		// Allow the setting of the orthographic width
		if (ImGui::SliderFloat("Orthographic Width", &orthoWidth, 1.0f, 10.0f))
			cameras[activeCamera]->SetOrthographicWidth(orthoWidth);
		break;
	}
}

void Game::ConstructLightUI()
{
	// Loop through all lights
	std::vector<Light> lights = gameRenderer->GetLightManager()->GetLights();

	// Edit ambient term
	XMFLOAT3 ambientTerm = gameRenderer->GetLightManager()->GetAmbientTerm();
	ImGui::ColorEdit3("Ambient Term", &ambientTerm.x);

	for (int i = 0; i < lights.size(); ++i)
	{
		// Push the current ID
		ImGui::PushID(i);

		// Get light data
		int type = lights[i].GetData().Type;
		XMFLOAT3 direction = lights[i].GetData().Direction;
		float range = lights[i].GetData().Range;
		XMFLOAT3 position = lights[i].GetData().Position;
		XMFLOAT3 color = lights[i].GetData().Color;
		float intensity = lights[i].GetData().Intensity;

		// Create the header
		std::string header = "Light " + std::to_string(i + 1);
		const char* cHeader = header.c_str();

		// List lights under headers
		if (ImGui::CollapsingHeader(cHeader))
		{
			// Show what type
			switch (type) 
			{
			case 0:
				ImGui::Text("Directional Light");
				ImGui::DragFloat3("Direction", &direction.x);
				ImGui::ColorEdit3("Color", &color.x);
				ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f);
				break;

			case 1:
				ImGui::Text("Point Light");
				ImGui::DragFloat3("Direction", &direction.x);
				ImGui::SliderFloat("Range", &range, 0.0f, 100.0f);
				ImGui::DragFloat3("Position", &position.x);
				ImGui::ColorEdit3("Color", &color.x);
				ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f);
				break;

			case 2:
				ImGui::Text("Spot Light");
				break;
			}
		}

		// Set light data
		lights[i].SetDirection(direction);
		lights[i].SetColor(color);
		lights[i].SetIntensity(intensity);

		// Set positional data for point/spot lights
		lights[i].SetPosition(position);
		lights[i].SetRange(range);

		// Pop the current ID
		ImGui::PopID();
	}

	// Set ambient term
	gameRenderer->GetLightManager()->SetAmbientTerm(ambientTerm);

	// Set lights
	gameRenderer->GetLightManager()->SetLights(lights);
}

void Game::ConstructMaterialsUI()
{
	for (auto& mat : materials)
	{
		// Get the key
		const char* key = mat.first.c_str();

		// Push the ID
		ImGui::PushID(key, key);

		// Retrieve data
		XMFLOAT3 colorTint = mat.second->GetColorTint();
		float roughness = mat.second->GetRoughness();
		float offset = mat.second->GetOffset();
		float scale = mat.second->GetScale();

		// Create a header with the key (name) as the string
		if (ImGui::CollapsingHeader(key))
		{
			// Display data to edit
			ImGui::ColorEdit3("Color Tint", &colorTint.x);
			ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
			ImGui::SliderFloat("Offset", &offset, 0.0f, 1.0f);
			ImGui::InputFloat("Scale", &scale);
		}

		// Set data
		mat.second->SetColorTint(colorTint);
		mat.second->SetRoughness(roughness);
		mat.second->SetOffset(offset);
		mat.second->SetScale(scale);

		// Pop the ID
		ImGui::PopID();
	}
}
