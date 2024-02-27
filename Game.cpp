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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
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

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
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

	// Create a renderer
	gameRenderer = std::make_shared<GameRenderer>(this->swapChain,
		this->device, this->context, this->backBufferRTV, this->depthBufferDSV);

	// Initialize the renderer
	gameRenderer->Init();

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create temporary variables to represent colors
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Create the triangle mesh
	{
		// Create vertices and indices
		Vertex vertices[] =
		{
			{ XMFLOAT3(+0.25f, -0.25f, +0.0f), red},
			{ XMFLOAT3(-0.25f, -0.25f, +0.0f), red},
			{ XMFLOAT3(+0.0f, +0.25f, +0.0f), blue}
		};
		unsigned int indices[] = { 0, 1, 2 };

		// Create the triangle mesh
		meshes.push_back(std::make_shared<Mesh>(this->context, this->swapChain, this->device, vertices, indices, 3, 3));
	}
	
	// Create the square mesh
	{
		// Create vertices and indices
		Vertex vertices[] =
		{
			{XMFLOAT3(+0.4f, +0.8f, +0.0f), blue},
			{XMFLOAT3(+0.8f, +0.8f, +0.0f), blue},
			{XMFLOAT3(+0.8f, +0.4f, +0.0f), green},
			{XMFLOAT3(+0.4f, +0.4f, +0.0f), green},
		};
		unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

		// Create the square mesh
		meshes.push_back(std::make_shared<Mesh>(this->context, this->swapChain, this->device, vertices, indices, 4, 6));
	}

	// Create the hexagon mesh
	{
		// Create vertices and indices
		Vertex vertices[] =
		{
			{XMFLOAT3(-0.825f, +0.9f, +0.0f), blue},
			{XMFLOAT3(-0.675f, +0.9f, +0.0f), red},
			{XMFLOAT3(-0.6f, +0.7f, +0.0f), red},
			{XMFLOAT3(-0.675f, +0.5f, +0.0f), green},
			{XMFLOAT3(-0.825f, +0.5f, +0.0f), green},
			{XMFLOAT3(-0.9f, +0.7f, +0.0f), blue},
		};
		unsigned int indices[] = { 0, 1, 2, 2, 3, 4, 4, 5, 2, 2, 5, 0 };

		// Create the square mesh
		meshes.push_back(std::make_shared<Mesh>(this->context, this->swapChain, this->device, vertices, indices, 6, 12));
	}
}

// --------------------------------------------------------
// Create the game entities
// --------------------------------------------------------
void Game::CreateEntities()
{
	// Create entities and add them to the list
	entities.push_back(std::make_shared<GameEntity>(meshes[0]));
	entities.push_back(std::make_shared<GameEntity>(meshes[0]));
	entities.push_back(std::make_shared<GameEntity>(meshes[1]));
	entities.push_back(std::make_shared<GameEntity>(meshes[2]));
	entities.push_back(std::make_shared<GameEntity>(meshes[2]));
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
	}

	// End the "Inspector" window
	ImGui::End();
}

// --------------------------------------------------------
// Update the game entities
// --------------------------------------------------------
void Game::UpdateEntities(const float& deltaTime, const float& totalTime)
{
	// Rotate all entities
	for (int i = 0; i < entities.size(); ++i)
	{
		entities[i]->GetTransform()->SetRotation(0, 0, totalTime);
	}

	// Scale the first and last entity
	float scale = (float)sin(totalTime) * 0.5f + 1.0f;
	entities[0]->GetTransform()->SetScale(scale, scale, scale);
	entities[4]->GetTransform()->SetScale(scale, scale, scale);

	// Move the middle entities
	entities[1]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	entities[2]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	entities[3]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
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

	// Update entities
	UpdateEntities(deltaTime, totalTime);

	// Update Buffers
	gameRenderer->Update(entities);

	// Update camera
	cameras[activeCamera]->Update(deltaTime);

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
	}
}

// --------------------------------------------------------
// Construct the Shaders ImGUI Tab
// --------------------------------------------------------
void Game::ConstructShadersUI()
{
	// Get the renderer data
	VertexShaderData vsData = gameRenderer->GetVSData();

	// World Matrix display
	ImGui::Text("World Matrix");
	ImGui::InputFloat4("", &vsData.world._11);
	ImGui::InputFloat4("", &vsData.world._21);
	ImGui::InputFloat4("", &vsData.world._31);
	ImGui::InputFloat4("", &vsData.world._41);

	ImGui::NewLine();

	// Color picker
	ImGui::Text("Shader Color Tint");
	ImGui::ColorEdit4("Color Tint", &vsData.colorTint.x);

	// Color reset button
	if (ImGui::Button("Reset Color Tint"))
		vsData.colorTint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Reset the data
	gameRenderer->SetVSData(vsData);
}

// --------------------------------------------------------
// Construct the Entities ImGUI Tab
// --------------------------------------------------------
void Game::ConstructEntitiesUI()
{
	// Loop through the entities
	for (int i = 0; i < entities.size(); ++i)
	{
		XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
		XMFLOAT3 pyrRotation = entities[i]->GetTransform()->GetPitchYawRoll();
		XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();
		unsigned int meshCount = entities[i]->GetMesh()->GetIndexCount();

		// Create the header
		std::string header = "Entity " + std::to_string(i);
		const char* cHeader = header.c_str();

		// List entities under headers
		if (ImGui::CollapsingHeader(cHeader))
		{
			ImGui::DragFloat3("Position", &position.x);
			ImGui::DragFloat3("Rotation", &pyrRotation.x);
			ImGui::DragFloat3("Scale", &scale.x);
			ImGui::Text("Mesh Index Count: %d", meshCount);
		}

		// Set transform
		entities[i]->GetTransform()->SetPosition(position);
		entities[i]->GetTransform()->SetRotation(pyrRotation);
		entities[i]->GetTransform()->SetScale(scale);
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

	// Get the projection type
	ProjectionType projectionType = cameras[activeCamera]->GetProjectionType();
	
	// Allow the setting of the basic transform variables
	if (ImGui::DragFloat3("Camera Position", &position.x, 0.01f))
		cameras[activeCamera]->GetTransform()->SetPosition(position);
	if (ImGui::DragFloat3("Camera Rotation (Radians, PYR)", &rotation.x, 0.01f))
		cameras[activeCamera]->GetTransform()->SetRotation(rotation);

	// Allow the setting of the near and far clip variables
	if (ImGui::SliderFloat("Near Clip Distance", &nearClip, 0.001, 1.0f))
		cameras[activeCamera]->SetNearClip(nearClip);
	if (ImGui::SliderFloat("Far Clip Distance", &farClip, 10.0f, 1000.0f))
		cameras[activeCamera]->SetFarClip(farClip);

	// Allow the chaging o the projection type
	int typeIndex = (int)projectionType;
	if (ImGui::Combo("Projection Type", &typeIndex, "Perspective\0Orthographic"))
	{
		projectionType = (ProjectionType)typeIndex;
		cameras[activeCamera]->SetProjectionType(projectionType);
	}

	// Show different things depending on the projection type
	if (projectionType == ProjectionType::Perspective)
	{
		// Get the field of view and convert it to degrees
		float fieldOfView = cameras[activeCamera]->GetFieldOfView() * 180.0f / XM_PI;

		// Allow the setting of the field of view
		if (ImGui::SliderFloat("Field of View (Degrees)", &fieldOfView, 0.01f, 180.0f))
			cameras[activeCamera]->SetFieldOfView(fieldOfView * XM_PI / 180.0f); // Need to convert back to radians
	}
	else
	{
		// Get the orthographic width
		float orthoWidth = cameras[activeCamera]->GetOrthographicWidth();

		// Allow the setting of the orthogrpahic width
		if (ImGui::SliderFloat("Orthographic Width", &orthoWidth, 1.0f, 10.0f))
			cameras[activeCamera]->SetOrthographicWidth(orthoWidth);
	}
}