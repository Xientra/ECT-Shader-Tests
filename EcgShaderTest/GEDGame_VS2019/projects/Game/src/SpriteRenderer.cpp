#include "SpriteRenderer.h"
#include <DDSTextureLoader.h>

#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}



SpriteRenderer::SpriteRenderer(const vector<wstring>& textureFilenames)
{
	m_textureFilenames = textureFilenames;
	m_pEffect = nullptr;
	//m_spriteSRV = ; // this is just an empty vector
	m_spriteCountMax = 0;
	m_pVertexBuffer = nullptr;
	m_pInputLayout = nullptr;
}

SpriteRenderer::~SpriteRenderer()
{

}

HRESULT SpriteRenderer::reloadShader(ID3D11Device* pDevice) // TODO I HAVE NO IDEA WHAT THIS ACTUALLY DOES
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"shader\\SpriteRenderer.fxo"));

	ifstream is(path, ios_base::binary);
	is.seekg(0, ios_base::end);
	streampos pos = is.tellg();
	is.seekg(0, ios_base::beg);
	vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();

	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	SAFE_RELEASE(pDevice);

	return hr;
}

void SpriteRenderer::releaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::create(ID3D11Device* pDevice)
{
	HRESULT hr;

	#pragma region create empty vertex buffer

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = 1024 * sizeof(SpriteVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	V(pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

	#pragma endregion


	#pragma region create and bind input layput

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		// semantic, [semanticIndex], format,		[input slot],  [byteOffset],			[inputSlotClass],		[step rate]
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTUREINDEX",   0, DXGI_FORMAT_R32_SINT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// number of elements in layout array
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	
	// Get Pass
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->GetDesc(&passDesc);

	// create input layout with pass and definition
	V_RETURN(pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout));

	#pragma endregion


	// Set Shader Variables
	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", m_ViewProjectionEV);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraUp", m_CameraWorldUp);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraRight", m_CameraWorldRight);

	// set the global SRV variables to the ones from the shader
	for (int i = 0; i < m_textureFilenames.size(); i++)
	{
		ID3D11ShaderResourceView* srv;
		const wchar_t* wcharPath = wstring(m_textureFilenames.at(i).begin(), m_textureFilenames.at(i).end()).c_str();
		V_RETURN(DirectX::CreateDDSTextureFromFile(pDevice, wcharPath, nullptr, &srv));
		m_spriteSRV.push_back(srv);
	}

	// get effect SRV from shader
	SAFE_GET_RESOURCE(m_pEffect, "g_GatlingSprite", m_GatlingSprite);
	SAFE_GET_RESOURCE(m_pEffect, "g_PlasmaSprite", m_PlasmaSprite);


	return hr;
}

void SpriteRenderer::destroy()
{
	SAFE_RELEASE(m_pEffect);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);


	SAFE_RELEASE(m_ViewProjectionEV);
	SAFE_RELEASE(m_CameraWorldUp);
	SAFE_RELEASE(m_CameraWorldRight);


	// release SRV list
	for (int i = 0; i < m_spriteSRV.size(); i++)
		SAFE_RELEASE(m_spriteSRV.at(i));

	// release effect SRV from shader
	SAFE_RELEASE(m_GatlingSprite);
	SAFE_RELEASE(m_PlasmaSprite);
}

void SpriteRenderer::renderSprites(ID3D11DeviceContext* context, const vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera)
{

	#pragma region fill/update vertex buffer

	D3D11_BOX box;
	box.left = 0;
	box.right = sprites.size() * sizeof(SpriteVertex);
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;

	context->UpdateSubresource(m_pVertexBuffer, 0, &box, &sprites[0], 0, 0);

	#pragma endregion

	#pragma region set input assembler stage

	// set Vertex Buffer
	ID3D11Buffer* vbs[] = { m_pVertexBuffer };
	unsigned int strides[] = { sizeof(SpriteVertex) };
	unsigned int offsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// set input layout
	context->IASetInputLayout(m_pInputLayout);

	// set topology
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	#pragma endregion


	// set shader variables
	DirectX::XMMATRIX viewProjection = camera.GetViewMatrix() * camera.GetProjMatrix();
	m_ViewProjectionEV->SetMatrix((float*)&(viewProjection));
	m_CameraWorldUp->SetFloatVector((float*)&(camera.GetWorldUp()));
	m_CameraWorldRight->SetFloatVector((float*)&(camera.GetWorldRight()));


	// set SRV variables (textures) in the shader
	m_GatlingSprite->SetResource(m_spriteSRV.at(0));
	m_PlasmaSprite->SetResource(m_spriteSRV.at(1));

	// apply pass
	m_pEffect->GetTechniqueByName("Render")->GetPassByName("P0")->Apply(0, context);

	context->Draw(sprites.size(), 0);

	// TODO "Don’t forget to release the context, or you will get a “non-zero reference count” warning
	//	when exiting your program", they said...
}
