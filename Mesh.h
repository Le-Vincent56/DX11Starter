#pragma once
#include "d3d11.h"
#include <wrl/client.h>
#include <DirectXMath.h>
#include "Vertex.h"
#include <vector>

class Mesh
{
private:
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain>	swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int numVertices;
	unsigned int numIndices;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

public:
	Mesh(Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
		Microsoft::WRL::ComPtr<ID3D11Device> _device, 
		Vertex* meshVertices, unsigned int* meshIndices, unsigned int numVertices, unsigned int numIndices);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	std::vector<Vertex> GetVertices();
	std::vector<unsigned int> GetIndices();
	unsigned int GetVertexCount();
	unsigned int GetIndexCount();
	void Draw();
};