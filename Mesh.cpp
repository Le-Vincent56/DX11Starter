#include "Mesh.h"

Mesh::Mesh(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_context,
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain,
	Microsoft::WRL::ComPtr<ID3D11Device> _device,
	Vertex* meshVertices, unsigned int* meshIndices, unsigned int numVertices, unsigned int numIndices)
	: context(_context), swapChain(_swapChain), device(_device)
{
	// Create a vertex buffer
	{
		// Fill the buffer struct
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		vbd.ByteWidth = sizeof(Vertex) * 3;       // 3 = number of vertices in the buffer
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		// Specify the initial data for the buffer
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = meshVertices; // pSysMem = Pointer to System Memory

		// Create the vertex buffer
		this->device->CreateBuffer(&vbd, &initialVertexData, this->vertexBuffer.GetAddressOf());

		// Set the size of the vertices array
		this->numVertices = numVertices;

		// Add vertices to the vector
		for (int i = 0; i < numVertices; i++)
		{
			vertices.push_back(meshVertices[i]);
		}
	}

	// Create an index buffer
	{
		// Fill the buffer struct
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		ibd.ByteWidth = sizeof(unsigned int) * 3;	// 3 = number of indices in the buffer
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
		ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Specify the initial data for the
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = meshIndices; // pSysMem = Pointer to System Memory

		// Create the index buffer
		this->device->CreateBuffer(&ibd, &initialIndexData, this->indexBuffer.GetAddressOf());

		// Set the size of the index array
		this->numIndices = numIndices;

		// Add indices to the vector
		for (int i = 0; i < numIndices; i++)
		{
			indices.push_back(meshIndices[i]);
		}
	}
}

Mesh::~Mesh()
{
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return this->vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return this->indexBuffer;
}

std::vector<Vertex> Mesh::GetVertices()
{
	return this->vertices;
}

std::vector<unsigned int> Mesh::GetIndices()
{
	return this->indices;
}

unsigned int Mesh::GetVertexCount()
{
	return this->numVertices;
}

unsigned int Mesh::GetIndexCount()
{
	return this->numIndices;
}

void Mesh::Draw()
{
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	{
		// Set buffers in the input assembler (IA) stage
		//  - Do this ONCE PER OBJECT, since each object may have different geometry
		//  - For this demo, this step *could* simply be done once during Init()
		//  - However, this needs to be done between EACH DrawIndexed() call
		//     when drawing different geometry, so it's here as an example
		this->context->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &stride, &offset);
		this->context->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Tell Direct3D to draw
		//  - Begins the rendering pipeline on the GPU
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all currently set Direct3D resources (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		this->context->DrawIndexed(
			this->numIndices,     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	}
}
