#include "Terrain.h"

#include "GameEffect.h"
#include <SimpleImage.h>
//#include "SimpleImage/SimpleImage.h"
#include <DDSTextureLoader.h>
#include "DirectXTex.h"

#include "ConfigParser.h"

#include <direct.h>
#include <DirectXMath.h> 

using namespace GEDUtils;
using namespace DirectX;

// You can use this macro to access your height field
#define IDX(X,Y,WIDTH) ((X) + (Y) * (WIDTH))

XMFLOAT3 getNormal(SimpleImage heightmap, int x, int y);

int indexCount;
int terrainResolution;

Terrain::Terrain(void) :
	indexBuffer(nullptr),
	//vertexBuffer(nullptr),
	diffuseTexture(nullptr),
	diffuseTextureSRV(nullptr),
	debugSRV(nullptr)
{
}


Terrain::~Terrain(void)
{
}

HRESULT Terrain::create(ID3D11Device* device)
{
	HRESULT hr;

	// load debug texture
	/*
	V(DirectX::CreateDDSTextureFromFile(device, L"resources\\debug_green.dds", nullptr, &debugSRV));
	if (hr != S_OK) {
		MessageBoxA (NULL, "Could not load texture \"resources\\debug_green.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}
	*/

	// This buffer contains positions, normals and texture coordinates for one triangle
	/*
	float triangle[] = {
		// Vertex 0
		   -400.0f, 0.0f, -400.0f,  1.0f, // Position
		   0.0f,    1.0f,    0.0f,  0.0f, // Normal
		   0.0f,    0.0f,                 // Texcoords

		// Vertex 1
		   400.0f,   0.0f, -400.0f, 1.0f, // Position
		   0.0f,     1.0f,    0.0f, 0.0f, // Normal
		   0.0f,     1.0f,                // Texcoords

		// Vertex 2
		   -400.0f, 0.0f,  400.0f,  1.0f, // Position
		   0.0f,    1.0f,    0.0f,  0.0f, // Normal
		   1.0f,    0.0f,                 // Texcoords

		 // Vertex 3
		   -400.0f, 0.0f,  -400.0f,  1.0f, // Position
		   0.0f,    1.0f,    0.0f,  0.0f, // Normal
		   1.0f,    1.0f,                 // Texcoords
	};
	*/

	// Visual Studio Path Change
	/*
	// if the project is started from visual studio the path has to be different
	if (IsDebuggerPresent())
	{
		resourcesPath = "..\\..\\x64\\Release\\resources\\";
		cout << "opened from visual studio" << endl;
	}
	*/


	// ---->> load images <<----

	string resourcesPath = "resources\\";

	#pragma region load heightmap as SimpleImage
	// load heightmap
	string terrainPath = configParserInstance.getTerrainHeightPath();
	SimpleImage heightmapImage = SimpleImage((resourcesPath + terrainPath).c_str());
	#pragma endregion

	#pragma region load and create normal map

	string normalPath = configParserInstance.getTerrainNormalPath();
	string path = resourcesPath + normalPath;
	const wchar_t* wcharNormalPath = wstring(path.begin(), path.end()).c_str();
	V(DirectX::CreateDDSTextureFromFile(device, wcharNormalPath, nullptr, &normalBufferSRV));

	if (hr != S_OK)
	{
		MessageBoxA(NULL, "Could not load texture \"resources\\normalPath\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	#pragma endregion

	#pragma region load and create color texture
	// Load color texture (color map)
	// TODO: Insert your code to load the color texture and create
	// the texture "diffuseTexture" as well as the shader resource view
	// "diffuseTextureSRV"

	// Load texturemap / colormap
	string colorPath = configParserInstance.getTerrainColorPath();
	string s = resourcesPath + colorPath;
	const wchar_t* p = wstring(s.begin(), s.end()).c_str();
	V(DirectX::CreateDDSTextureFromFile(device, p, nullptr, &diffuseTextureSRV));

	if (hr != S_OK)
	{
		MessageBoxA(NULL, "Could not load texture \"resources\\colorPath\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}
	#pragma endregion


	// ---->> create heightmap buffer and shader resource view <<----

	#pragma region create array from heightmap image

	terrainResolution = heightmapImage.getHeight();
	int heightmapArrayLength = heightmapImage.getHeight() * heightmapImage.getWidth();
	float* heightmapArray = new float[heightmapArrayLength];

	for (int y = 0; y < heightmapImage.getHeight(); y++)
	{
		for (int x = 0; x < heightmapImage.getWidth(); x++)
		{
			heightmapArray[IDX(x, y, heightmapImage.getWidth())] = heightmapImage.getPixel(x, y);
		}
	}
	#pragma endregion

	#pragma region create heightmap buffer and heightmap shader resource view

	// create heightmap Buffer

	D3D11_SUBRESOURCE_DATA height_data;
	height_data.pSysMem = heightmapArray;
	height_data.SysMemSlicePitch = 0;
	height_data.SysMemPitch = sizeof(float);

	D3D11_BUFFER_DESC height_desc;
	height_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	height_desc.Usage = D3D11_USAGE_DEFAULT;
	height_desc.CPUAccessFlags = 0;
	height_desc.MiscFlags = 0;
	height_desc.ByteWidth = heightmapArrayLength * sizeof(float);

	V(device->CreateBuffer(&height_desc, &height_data, &heightBuffer));



	// create shader rescource view for heightmap buffer

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderRescDesc;
	shaderRescDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderRescDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderRescDesc.Buffer.FirstElement = 0;
	shaderRescDesc.Buffer.NumElements = terrainResolution * terrainResolution;

	V(device->CreateShaderResourceView(heightBuffer, &shaderRescDesc, &heightBufferSRV));
	#pragma endregion

	delete[] heightmapArray;


	// ---->> generate vertex buffer <<----

	#pragma region vertex buffer generation

	/*
	TODO: Replace this vertex array (triangle) with an array (or vector)
	which contains the vertices of your terrain.
	Calculate position, normal and texture coordinates according to your height field and
	the dimensions of the terrain specified by the ConfigParser


	Note 1: The normal map that you created last week will not be used
	in this assignment (Assignment 4). It will be of use in later assignments

	Note 2: For each vertex 10 floats are stored. Do not use another
	layout.

	Note 3: In the coordinate system of the vertex buffer (output):
	x = east,    y = up,    z = south,          x,y,z in [0,1] (float)
	*/

	/*
	// create vertex buffer from hightmap
	int vertexBufferArrLength = heightmapImage.getHeight() * heightmapImage.getWidth() * 10;
	float* vertexBufferArr = new float[vertexBufferArrLength];

	// create vertex buffer
	for (int y = 0; y < heightmapImage.getHeight(); y++)
	{
		for (int x = 0; x < heightmapImage.getWidth(); x++)
		{
			int _x = x * 10;
			int _y = y * 10;

			// x, y, z, w, nx, ny, nz, nw, u, v

			float res = heightmapImage.getWidth();

			// set pos
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 0] = ((float)x / res - 0.5) * res;//x + (heightmapImage.getWidth() / 2);
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 1] = heightmapImage.getPixel(x, y) * heightmapImage.getHeight();
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 2] = ((float)y / res - 0.5) * res;// + (heightmapImage.getHeight() / 2);
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 3] = 1;

			// recalculate normal
			XMFLOAT3 normalValues = getNormal(heightmapImage, x, y);

			// magic normal scaling
			XMMATRIX matNormalScaling = XMMatrixScaling(heightmapImage.getWidth(), heightmapImage.getWidth(), heightmapImage.getWidth());
			matNormalScaling = XMMatrixTranspose(XMMatrixInverse(nullptr, matNormalScaling));

			XMVECTOR vNormal = XMVectorSet(normalValues.x, normalValues.y, normalValues.z, 1);
			vNormal = XMVector4Transform(vNormal, matNormalScaling);
			vNormal = XMVector3Normalize(vNormal);


			// set normals
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 4] = XMVectorGetX(vNormal);
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 5] = XMVectorGetZ(vNormal); // <<------ recalculate!!!!!
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 6] = XMVectorGetY(vNormal);
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 7] = 0;

			// set uv
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 8] = (float)x / heightmapImage.getWidth();
			vertexBufferArr[IDX(_x, y, heightmapImage.getWidth() * 10) + 9] = (float)y / heightmapImage.getHeight();
		}
	}
	*/
	#pragma endregion

	#pragma region create vertex buffer
	/*
	D3D11_SUBRESOURCE_DATA id;
	id.pSysMem = vertexBufferArr;
	id.SysMemSlicePitch = 0;
	//id.pSysMem = &triangle[0];
	id.SysMemPitch = 10 * sizeof(float); // Stride

	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	// TODO: Change bd.ByteWidth s.t. (so that?) it fits the size of your vertex buffer
	//bd.ByteWidth = sizeof(triangle); //The size in bytes of the triangle array
	bd.ByteWidth = vertexBufferArrLength * sizeof(float); //sizeof(vertexBufferArr);


	V(device->CreateBuffer(&bd, &id, &vertexBuffer)); // http://msdn.microsoft.com/en-us/library/ff476899%28v=vs.85%29.aspx

	*/
	#pragma endregion

	// delete[] vertexBufferArr;


	// ---->> generate index buffer <<----

	#pragma region index buffer generation

	// TODO: Insert your code to create the index buffer

	// index buffer variables
	int vertexArrSize = heightmapImage.getHeight();

	int squaresSize = vertexArrSize - 1;
	//int triangles = squares * 2;

	int indexBufferArrWidth = squaresSize * 6;
	int indexBufferArrHight = squaresSize;
	int indexBufferArrLength = indexBufferArrWidth * indexBufferArrHight;

	int* indexBufferArr = new int[indexBufferArrLength] {};

	// set global variable for drawIndexes()
	indexCount = indexBufferArrLength;

	// create index buffer
	// go thorught the vertex buffer array - 1 (for all squares once)
	for (int y = 0; y < squaresSize; y++)
	{
		for (int x = 0; x < squaresSize; x++)
		{
			int _x = x * 6; // the index for the index buffer array

			// triange 1
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 0] = IDX(x, y, vertexArrSize);
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 1] = IDX(x + 1, y, vertexArrSize);
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 2] = IDX(x, y + 1, vertexArrSize);

			// triange 2                                        
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 3] = IDX(x, y + 1, vertexArrSize);
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 4] = IDX(x + 1, y, vertexArrSize);
			indexBufferArr[IDX(_x, y, indexBufferArrWidth) + 5] = IDX(x + 1, y + 1, vertexArrSize);

			/*
			example

			0 1 2
			3 4 5
			6 7 8

			013314 124425
			346647 457758
			*/
		}
	}

	/*
	for (int y = 0; y < indexBufferArrHight; y++)
	{
		for (int x = 0; x < indexBufferArrWidth; x++)
		{
			cout << indexBufferArr[IDX(x, y, indexBufferArrWidth)] << " ";
		}
		cout << endl;
	}
	*/
	#pragma endregion

	#pragma region create index buffer

	// create index buffer for vertex buffer
	/*
	// Create and fill description
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof(unsigned int) * indices.size();
	bd.ByteWidth = sizeof(unsigned int) * indexBufferArrLength;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // <<---------------------- MAIN DIFFERENCE
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	// Define initial data
	ZeroMemory(&id, sizeof(id));
	//id.pSysMem = &indices[0];
	id.pSysMem = indexBufferArr;

	// Create Index Buffer
	V(device->CreateBuffer(&bd, &id, &indexBuffer));
	*/

	// create index buffer for heightmap buffer

	// Create and fill description  
	ZeroMemory(&height_desc, sizeof(height_desc));
	height_desc.Usage = D3D11_USAGE_DEFAULT;
	height_desc.ByteWidth = sizeof(unsigned int) * indexBufferArrLength;
	height_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	height_desc.CPUAccessFlags = 0;
	height_desc.MiscFlags = 0;

	// Define initial data 
	ZeroMemory(&height_data, sizeof(height_data));
	height_data.pSysMem = indexBufferArr;

	// Create Index Buffer 
	V(device->CreateBuffer(&height_desc, &height_data, &indexBuffer));

	#pragma endregion

	delete[] indexBufferArr;

	return hr;
}


void Terrain::destroy()
{
	//SAFE_RELEASE(vertexBuffer); TODO DELETE
	// TODO: Release the index buffer
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(debugSRV);

	// TODO: Release the terrain's shader resource view and texture
	SAFE_RELEASE(diffuseTexture);
	SAFE_RELEASE(diffuseTextureSRV);

	// for the normal map
	SAFE_RELEASE(normalBuffer);
	SAFE_RELEASE(normalBufferSRV);

	// for the terrain heightmap
	SAFE_RELEASE(heightBuffer);
	SAFE_RELEASE(heightBufferSRV);
}


void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	context->IASetInputLayout(nullptr);

	// Bind the terrain vertex buffer to the input assembler stage 
	//ID3D11Buffer* vbs[] = { vertexBuffer, };
	//unsigned int strides[] = { 10 * sizeof(float), }, offsets[] = { 0, };
	ID3D11Buffer* vbs[] = { nullptr };
	unsigned int strides[] = { 0 }, offsets[] = { 0, };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// TODO: Bind the terrain index buffer to the input assembler stage
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);



	// Tell the input assembler stage which primitive topology to use
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// TODO: Bind the SRV of the terrain diffuse texture to the effect variable
	// (instead of the SRV of the debug texture)
	V(g_gameEffect.diffuseEV->SetResource(diffuseTextureSRV));

	V(g_gameEffect.heightmap->SetResource(heightBufferSRV));
	V(g_gameEffect.normalmap->SetResource(normalBufferSRV));

	V(g_gameEffect.resolution->SetInt(terrainResolution));

	// Apply the rendering pass in order to submit the necessary render state changes to the device
	V(pass->Apply(0, context));

	// Draw
	// TODO: Use DrawIndexed to draw the terrain geometry using as shared vertex list
	// (instead of drawing only the vertex buffer)
	//context->Draw(512 * 512, 0);
	context->DrawIndexed(indexCount, 0, 0);
}

XMFLOAT3 getNormal(SimpleImage heightmap, int x, int y)
{
	int resolution = heightmap.getHeight();

	int div = 2;

	float xPrev;

	if (x - 1 >= 0)
	{
		xPrev = heightmap.getPixel(x - 1, y);
	}
	else
	{
		xPrev = heightmap.getPixel(x, y);
		div = 1;
	}

	float xNext;
	if (x + 1 < resolution)
	{
		xNext = heightmap.getPixel(x + 1, y);
	}
	else
	{
		xNext = heightmap.getPixel(x, y);
		div = 1;
	}

	xNext *= resolution;
	xPrev *= resolution;

	float xDiv = (xNext - xPrev) / div;

	div = 2;

	float yPrev;
	if (y - 1 >= 0)
	{
		yPrev = heightmap.getPixel(x, y - 1);
	}
	else
	{
		yPrev = heightmap.getPixel(x, y);
		div = 1;
	}

	float yNext;
	if (y + 1 < resolution)
	{
		yNext = heightmap.getPixel(x, y + 1);
	}
	else
	{
		yNext = heightmap.getPixel(x, y);
		div = 1;
	}

	yNext *= resolution;
	yPrev *= resolution;

	float yDiv = (yNext - yPrev) / div;


	float nX = -xDiv;
	float nY = -yDiv;
	float nZ = 1;

	float length = sqrt(nX * nX + nY * nY + nZ * nZ);

	return XMFLOAT3(nX / length, nY / length, nZ / length);
}
