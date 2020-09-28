#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <map>
#include <list>


#include "dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "d3dx11effect.h"

#include "Terrain.h"
#include "GameEffect.h"
#include "Mesh.h"

#include "debug.h"

#include "ConfigParser.h"
#include "SpriteRenderer.h"

#include <random>
#include <time.h>
#include "Enemy.h"


// Help macros
#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

struct Gun
{
	GunConfiguration* config;

	float cooldown = 0;

	bool firing = false;
};

struct Projectile
{
	XMVECTOR position;

	XMVECTOR direction;

	Gun* origin;

	void move(float fElapsedTime)
	{
		// apply gravity
		XMVectorSetY(direction, XMVectorGetY(direction) - origin->config->gravity * fElapsedTime);

		// update position
		position += direction * origin->config->speed * fElapsedTime;
	}
};

// Camera
struct CAMERAPARAMS
{
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}                                       g_cameraParams;
float                                   g_cameraMoveScaler = 1000.f;
float                                   g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_camera;               // A first person camera

// User Interface
CDXUTDialogResourceManager              g_dialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_settingsDlg;          // Device settings dialog
CDXUTTextHelper* g_txtHelper = NULL;
CDXUTDialog                             g_hud;                  // dialog for standard controls
CDXUTDialog                             g_sampleUI;             // dialog for sample specific controls

//ID3D11InputLayout*                      g_terrainVertexLayout; // Describes the structure of the vertex buffer to the input assembler stage DONE DELETE


bool                                    g_terrainSpinning = false;
XMMATRIX                                g_terrainWorld; // object- to world-space transformation


// Scene information
XMVECTOR                                g_lightDir;
Terrain									g_terrain;

GameEffect								g_gameEffect; // CPU part of Shader

SpriteRenderer* g_SpriteRenderer;

// Meshes
//Mesh*                                   g_cockpitMesh = nullptr;
map<string, Mesh*>                      g_Meshes;

//enemies
list<Enemy*>                            g_EnemyInstances;

// projectiles
list<Projectile>						g_Projectiles;

// guns
Gun										g_GatlingGun;
Gun										g_PlasmaGun;

// spawn variables
float                                   g_SpawnInterval;
float                                   g_SpawnTimer;
float									g_startupTime = 0; // TODO

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo*, UINT, const CD3D11EnumDeviceInfo*,
	DXGI_FORMAT, bool, void*);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

void InitApp();
void DeinitApp();
void RenderText();

void SpawnEnemy();
XMVECTOR GetRandomPositionOnCircle(float radius);

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// Old Direct3D Documentation:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

	// DXUT Documentaion:
	// Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT

	// New Direct3D Documentaion (just for reference, use old documentation to find explanations):
	// http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx


	// Initialize COM library for windows imaging components
	/*HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (hr != S_OK)
	{
		MessageBox(0, L"Error calling CoInitializeEx", L"Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}*/


	// Set DXUT callbacks
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	//DXUTSetIsInGammaCorrectMode(false);

	InitApp();
	DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"TODO: Insert Title Here"); // You may change the title

	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_0, true, 1280, 720);

	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();
	DeinitApp();

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	// Parse the config file

	V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	char pathA[MAX_PATH];
	size_t size;
	wcstombs_s(&size, pathA, path, MAX_PATH);

	// L"parTrailPlasmaDiffuse.png"
	// L"parTrailGatlingDiffuse.png"


	// TODO: Parse your config file specified by "pathA" here
	configParserInstance.load(pathA);


	// create mesh from loaded paths
	string resourcesPath = "resources\\";

	// get meshes from config parser
	for (int i = 0; i < configParserInstance.meshes.size(); i++)
	{
		MeshData md = configParserInstance.meshes.at(i);

		string meshFilePath = resourcesPath + md.meshFile;
		string diffuseTexturePath = resourcesPath + md.diffuseTexture;
		string specularTexturePath = resourcesPath + md.specularTexture;
		string glowTexturePath = resourcesPath + md.glowTexture;

		g_Meshes.insert(pair<string, Mesh*>(md.meshIdentifier, new Mesh(meshFilePath, diffuseTexturePath, specularTexturePath, glowTexturePath)));
	}


	// create SpriteRenderer
	vector<wstring> textureFilenames{};
	for (int i = 0; i < configParserInstance.particlePaths.size(); i++)
	{
		string fullPath = resourcesPath + configParserInstance.particlePaths.at(i);
		textureFilenames.push_back(wstring(fullPath.begin(), fullPath.end()));
	}

	g_SpriteRenderer = new SpriteRenderer(textureFilenames);


	// load gun configurations
	g_GatlingGun.config = &configParserInstance.gunConfigurations.at(0);
	g_PlasmaGun.config = &configParserInstance.gunConfigurations.at(1);

	// Intialize the user interface

	g_settingsDlg.Init(&g_dialogResourceManager);
	g_hud.Init(&g_dialogResourceManager);
	g_sampleUI.Init(&g_dialogResourceManager);

	g_hud.SetCallback(OnGUIEvent);
	int iY = 30;
	int iYo = 26;
	g_hud.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22);
	g_hud.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3);
	g_hud.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2);

	g_hud.AddButton(IDC_RELOAD_SHADERS, L"Reload shaders (F5)", 0, iY += 24, 170, 22, VK_F5);

	g_sampleUI.SetCallback(OnGUIEvent); iY = 10;
	iY += 24;
	g_sampleUI.AddCheckBox(IDC_TOGGLESPIN, L"Spin Lighting", 0, iY += 24, 125, 22, g_terrainSpinning);
}


void DeinitApp()
{
	// Every new in InitApp should be deleted in here.
	//SAFE_DELETE(g_cockpitMesh);
	SAFE_DELETE(g_SpriteRenderer);

	// deletes every Mesh/Object in the map
	for (map<string, Mesh*>::iterator it = g_Meshes.begin(); it != g_Meshes.end(); ++it)
	{
		SAFE_DELETE(it->second);
	}
	g_Meshes.clear();
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	g_txtHelper->Begin();
	g_txtHelper->SetInsertionPos(5, 5);
	g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
	g_txtHelper->DrawTextLine(DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
	g_txtHelper->DrawTextLine(DXUTGetDeviceStats());
	g_txtHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo*, UINT, const CD3D11EnumDeviceInfo*,
	DXGI_FORMAT, bool, void*)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pDeviceSettings);
	UNREFERENCED_PARAMETER(pUserContext);

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			DXUTDisplaySwitchingToREFWarning();
		}
	}
	//// Enable anti aliasing
	pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
	pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pBackBufferSurfaceDesc);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
	V_RETURN(g_dialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	V_RETURN(g_settingsDlg.OnD3D11CreateDevice(pd3dDevice));
	g_txtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15);

	V_RETURN(ReloadShader(pd3dDevice));


	V_RETURN(g_SpriteRenderer->create(pd3dDevice));





	// get player pos from config
	float playerPosX;
	float playerPosY;
	float playerPosZ;
	configParserInstance.getPlayerPosition(playerPosX, playerPosY, playerPosZ);


	// Initialize the camera
	//XMVECTOR vEye = XMVectorSet(0.0f, 400.0f, -500.0f, 0.0f);   // Camera eye is here
	XMVECTOR vEye = XMVectorSet(playerPosX, playerPosY, playerPosZ + 0.00001f, 0.0f);   // Camera eye is here //TODO: change Eye.y
	XMVECTOR vAt = XMVectorSet(-205.0f, 400.0f, 5.0f, 1.0f); // ... looking at this position
	g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "SV_POSITION",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",         0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);


	// Create the input layout
	D3DX11_PASS_DESC pd;
	V_RETURN(g_gameEffect.pass0->GetDesc(&pd));
	//V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, pd.pIAInputSignature, pd.IAInputSignatureSize, &g_terrainVertexLayout ) ); // TODO DELETE


	// Create the terrain
	V_RETURN(g_terrain.create(pd3dDevice));
	// TODO: You might pass a ConfigParser object to the create function.
	//       Therefore you can adjust the TerrainClass accordingly



	// create cockpit mesh
	V_RETURN(g_Meshes.at("Cockpit")->createInputLayout(pd3dDevice, g_gameEffect.meshPass1));
	//V_RETURN(g_Meshes.at("Cockpit")->create(pd3dDevice));

	// create all meshes
	for (map<string, Mesh*>::iterator it = g_Meshes.begin(); it != g_Meshes.end(); ++it)
	{
		V_RETURN(it->second->create(pd3dDevice));
	}


	return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	g_dialogResourceManager.OnD3D11DestroyDevice();
	g_settingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	//SAFE_RELEASE( g_terrainVertexLayout ); //TODO DELETE

	g_SpriteRenderer->destroy();

	// Destroy the terrain
	g_terrain.destroy();

	// destroy cockpit
	//g_cockpitMesh->destroyInputLayout();
	//g_cockpitMesh->destroy();

	/*
	// destroy meshes
	g_Meshes.at("Cockpit")->destroyInputLayout();
	g_Meshes.at("Cockpit")->destroy();
	g_Meshes.at("GatlingGunBase")->destroy();
	g_Meshes.at("GatlingGunTop")->destroy();
	g_Meshes.at("PlasmaGunBase")->destroy();
	g_Meshes.at("PlasmaGunTop")->destroy();
	g_Meshes.at("Fallbirch01")->destroy();
	g_Meshes.at("Tower")->destroy();
	g_Meshes.at("JufSpaceship")->destroy();
	g_Meshes.at("LupSpaceship")->destroy();
	g_Meshes.at("ManSpaceship")->destroy();
	*/

	for (map<string, Mesh*>::iterator it = g_Meshes.begin(); it != g_Meshes.end(); ++it)
	{
		it->second->destroy();
	}


	SAFE_DELETE(g_txtHelper);
	ReleaseShader();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pSwapChain);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// Intialize the user interface

	V_RETURN(g_dialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(g_settingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	g_hud.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_hud.SetSize(170, 170);
	g_sampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_sampleUI.SetSize(170, 300);

	// Initialize the camera

	g_cameraParams.aspect = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_cameraParams.fovy = 0.785398f;
	g_cameraParams.nearPlane = 1.f;
	g_cameraParams.farPlane = 5000.f;

	g_camera.SetProjParams(g_cameraParams.fovy, g_cameraParams.aspect, g_cameraParams.nearPlane, g_cameraParams.farPlane);
	g_camera.SetEnablePositionMovement(false);
	g_camera.SetRotateButtons(true, false, false);
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	g_camera.SetDrag(true);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
	assert(pd3dDevice != NULL);

	HRESULT hr;

	ReleaseShader();
	V_RETURN(g_gameEffect.create(pd3dDevice));

	V_RETURN(g_SpriteRenderer->reloadShader(pd3dDevice));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	g_gameEffect.destroy();
	g_SpriteRenderer->releaseShader();

}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);

	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_dialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Pass messages to settings dialog if its active
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_hud.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = g_sampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	// Use the mouse weel to control the movement speed
	if (uMsg == WM_MOUSEWHEEL)
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_cameraMoveScaler *= (1 + zDelta / 500.0f);
		if (g_cameraMoveScaler < 0.1f)
			g_cameraMoveScaler = 0.1f;
		g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	}

	// Pass all remaining windows messages to camera so it can respond to user input
	g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nChar);
	UNREFERENCED_PARAMETER(bKeyDown);
	UNREFERENCED_PARAMETER(bAltDown);
	UNREFERENCED_PARAMETER(pUserContext);


	//Enable position movement when the C-key is pressed
	if (nChar == 'C' && bKeyDown)
	{
		/* enable position movement here */
		g_camera.SetEnablePositionMovement(true);
	}

	// set gatling state
	if (nChar == 'R')
		g_GatlingGun.firing = bKeyDown;

	// set plasma state
	if (nChar == 'F')
		g_PlasmaGun.firing = bKeyDown;
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	UNREFERENCED_PARAMETER(nEvent);
	UNREFERENCED_PARAMETER(pControl);
	UNREFERENCED_PARAMETER(pUserContext);

	switch (nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		g_settingsDlg.SetActive(!g_settingsDlg.IsActive()); break;
	case IDC_TOGGLESPIN:
		g_terrainSpinning = g_sampleUI.GetCheckBox(IDC_TOGGLESPIN)->GetChecked();
		break;
	case IDC_RELOAD_SHADERS:
		ReloadShader(DXUTGetD3D11Device());
		break;
	}
}


void CreateProjectile(Gun& origin)
{
	XMVECTOR startPos = XMVectorSet(origin.config->projectileSpawn.x, origin.config->projectileSpawn.y, origin.config->projectileSpawn.z, 1);
	startPos = XMVector3Transform(startPos, g_camera.GetWorldMatrix());

	XMVECTOR direction = g_camera.GetWorldAhead();

	Projectile p = { startPos, direction, &origin };

	g_Projectiles.push_back(p);
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pUserContext);
	// Update the camera's position based on user input 
	g_camera.FrameMove(fElapsedTime);

	#pragma region manage terrain

	// Initialize the terrain world matrix
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx

	// Start with identity matrix
	g_terrainWorld = XMMatrixIdentity();

	// scale the terrainWorld matrix TODO
	XMMATRIX scaling = XMMatrixScaling(configParserInstance.getTerrainWidth(), configParserInstance.getTerrainHeight(), configParserInstance.getTerrainDepth());
	g_terrainWorld *= scaling;


	// Set the light vector
	g_lightDir = XMVectorSet(1, 1, 1, 0); // Direction to the directional light in world space    

	// spin terrain
	if (g_terrainSpinning)
	{
		// If spinng enabled, rotate the world matrix around the y-axis
		//g_terrainWorld *= XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
		g_lightDir = XMVector3Transform(g_lightDir, XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)));
	}

	g_lightDir = XMVector3Normalize(g_lightDir);


	#pragma endregion

	bool spawnProjectiles = false;
	#pragma region projectile management

	if (spawnProjectiles)
	{
		// manage projectile creation
		if (g_GatlingGun.firing && g_GatlingGun.cooldown <= 0)
		{
			CreateProjectile(g_GatlingGun);

			g_GatlingGun.cooldown = g_GatlingGun.config->fireCooldown;
		}
		if (g_GatlingGun.cooldown > 0)
			g_GatlingGun.cooldown -= fElapsedTime;

		if (g_PlasmaGun.firing && g_PlasmaGun.cooldown <= 0)
		{
			CreateProjectile(g_PlasmaGun);

			g_PlasmaGun.cooldown = g_PlasmaGun.config->fireCooldown;
		}
		if (g_PlasmaGun.cooldown > 0)
			g_PlasmaGun.cooldown -= fElapsedTime;


		// move projectiles
		for (list<Projectile>::iterator it = g_Projectiles.begin(); it != g_Projectiles.end(); it++)
		{
			Projectile p = *it;

			it->move(fElapsedTime);

			// destroy projectiles
			if (XMVectorGetX(XMVector3Length(p.position)) > configParserInstance.getOuterCircle())
				g_Projectiles.erase(it++);
		}
	}
	#pragma endregion

	bool spawnEnemies = false;
	#pragma region enemie management
	// spawn enemies
	if (spawnEnemies)
	{
		g_SpawnTimer -= fElapsedTime;
		if (g_SpawnTimer < 0)
		{
			g_SpawnTimer = configParserInstance.getSpawnInterval();
			SpawnEnemy();
		}

		// move enemies
		for (list<Enemy*>::iterator it = g_EnemyInstances.begin(); it != g_EnemyInstances.end(); it++)
		{
			Enemy* enemy = *it;
			Enemy e = *enemy;

			enemy->move(fElapsedTime);

			// delete enemies if they are too far away from the origin point
			if (XMVectorGetX(XMVector3Length(enemy->getPos())) > configParserInstance.getOuterCircle())
			{
				g_EnemyInstances.erase(it);
				SAFE_DELETE(enemy);
			}
		}
	}

	#pragma endregion

	#pragma region manage collision

	// check collision
	for (list<Projectile>::iterator pit = g_Projectiles.begin(); pit != g_Projectiles.end(); pit++)
	{
		for (list<Enemy*>::iterator eit = g_EnemyInstances.begin(); eit != g_EnemyInstances.end(); eit++)
		{
			Enemy e = **eit;
			Projectile p = *pit;

			float rad = configParserInstance.getEnemyTypeByName(e.getEnemyType())->size;

			// if collison, take dmg
			if (XMVectorGetX(XMVector3LengthSq(e.getPos() - p.position)) < rad * rad)
			{
				g_Projectiles.erase(pit++);

				if ((*eit)->takeDamage(p.origin->config->damage))
					g_EnemyInstances.remove(*eit++);
			}
		}
	}

	#pragma endregion

}

void SpawnEnemy()
{
	EnemyType enemy = configParserInstance.enemyObjects.at(rand() % configParserInstance.enemyObjects.size());

	float a = configParserInstance.getSpawnHeightMultiplierMin() * configParserInstance.getTerrainHeight();
	float b = configParserInstance.getSpawnHeightMultiplierMax() * configParserInstance.getTerrainHeight();

	float rndHeight = rand() % (int)b + (int)a;

	// get position on 2d circle
	XMVECTOR position2d = GetRandomPositionOnCircle(configParserInstance.getSpawnCircle());
	XMVECTOR target2d = GetRandomPositionOnCircle(configParserInstance.getInnerCircle());

	// make points 3d
	XMVECTOR position = XMVectorSet(XMVectorGetX(position2d), rndHeight, XMVectorGetZ(position2d), 0);
	XMVECTOR target = XMVectorSet(XMVectorGetX(target2d), rndHeight, XMVectorGetZ(target2d), 0);

	XMVECTOR velocity = enemy.speed * XMVector3Normalize(target - position);

	//cout << "position: " << XMVectorGetX(position) << " " << XMVectorGetY(position) << " " << XMVectorGetZ(position) << endl;
	//cout << "target: " << XMVectorGetX(target) << " " << XMVectorGetY(target) << " " << XMVectorGetZ(target) << endl;

	//cout << "height: " << XMVectorGetY(velocity) << endl;


	Enemy* enemyObj = new Enemy(enemy.typeIdentifier, enemy.hitpoints, position, velocity);
	g_EnemyInstances.push_back(enemyObj);
}

XMVECTOR GetRandomPositionOnCircle(float radius)
{
	int rndDegree = rand() % 360;
	float rndRad = DEG2RAD(rndDegree);


	return XMVectorSet(radius * sin(rndRad), 0, radius * cos(rndRad), 0);
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	UNREFERENCED_PARAMETER(pd3dDevice);
	UNREFERENCED_PARAMETER(fTime);
	UNREFERENCED_PARAMETER(pUserContext);

	HRESULT hr;

	// If the settings dialog is being shown, then render it instead of rendering the app's scene
	if (g_settingsDlg.IsActive())
	{
		g_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	//float clearColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	pd3dImmediateContext->ClearRenderTargetView(pRTV, clearColor);

	if (g_gameEffect.effect == NULL)
	{
		g_txtHelper->Begin();
		g_txtHelper->SetInsertionPos(5, 5);
		g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
		g_txtHelper->DrawTextLine(L"SHADER ERROR");
		g_txtHelper->End();
		return;
	}


	// Clear the depth stencil
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);


	// <<<<-------------------------------------------------------------------------------------------------------------------- Place 1
	#pragma region variables for shader tests

	ID3DX11EffectScalarVariable* g_startupTimeSV = g_gameEffect.effect->GetVariableByName("g_startupTime")->AsScalar();
	g_startupTimeSV->SetFloat(g_startupTime);
	g_startupTime += fElapsedTime;

	ID3DX11EffectVectorVariable* g_CameraForwardESV = g_gameEffect.effect->GetVariableByName("g_CameraForward")->AsVector();
	g_CameraForwardESV->SetFloatVector((float*)&(g_camera.GetWorldAhead()));

	g_camera.GetNearClip();

	#pragma endregion


	bool renderTerrain = false;
	#pragma region render terrain

	// Update variables that change once per frame
	XMMATRIX const view = g_camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	XMMATRIX const proj = g_camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
	XMMATRIX worldViewProj = g_terrainWorld * view * proj;

	V(g_gameEffect.worldEV->SetMatrix((float*)&g_terrainWorld));
	V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&worldViewProj));
	V(g_gameEffect.lightDirEV->SetFloatVector((float*)&g_lightDir));

	V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&XMMatrixTranspose(XMMatrixInverse(nullptr, g_terrainWorld))));

	//V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&g_terrainWorld));

	// Set input layout
	//pd3dImmediateContext->IASetInputLayout( g_terrainVertexLayout ); // TODO DELETE
	pd3dImmediateContext->IASetInputLayout(nullptr);

	// renderer terrain
	if (renderTerrain)
		g_terrain.render(pd3dImmediateContext, g_gameEffect.pass0);

	#pragma endregion

	bool renderCockpit = false;
	#pragma region Render CockpitObjects

	for (int i = 0; i < configParserInstance.cockpitObjects.size(); i++)
	{
		ObjectData data = configParserInstance.cockpitObjects.at(i);

		//Create transformation matrices
		XMMATRIX mRot = XMMatrixRotationX((float)DEG2RAD(data.xRotation)) * XMMatrixRotationY((float)DEG2RAD(data.yRotation)) * XMMatrixRotationZ((float)DEG2RAD(data.zRotation));
		XMMATRIX mTrans = XMMatrixTranslation(data.xTranslation, data.yTranslation, data.zTranslation);
		XMMATRIX mScale = XMMatrixScaling(data.scale, data.scale, data.scale);

		//Object to world space for cockpit (for lighting):
		// rotation first, then translation and then scaling, then transform
		// to camera position / rotation
		XMMATRIX mWorld = mRot * mTrans * mScale * g_camera.GetWorldMatrix();
		//XMMATRIX mWorld = mScale * mRot * mTrans * g_camera.GetWorldMatrix();

		//Object to clip space for cockpit (for rendering):
		XMMATRIX mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();
		// Note: mRot * mTrans * mScale * (*g_Camera.GetProjMatrix()) yields the
		// same result since GetWorldMatrix() is the inverse of GetViewMatrix()

		//Inverse transposed of world for transformation of normals
		XMMATRIX worldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));

		//set effect variables
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&worldNormals));
		V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&g_camera.GetEyePt()));


		//render cockpit object
		if (renderCockpit)
			g_Meshes.at(data.meshIdentifier)->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);
	}

	#pragma endregion

	#pragma region Render mesh objects

	for (int i = 0; i < configParserInstance.groundObjects.size(); i++)
	{
		ObjectData data = configParserInstance.groundObjects.at(i);

		//Create transformation matrices
		XMMATRIX mRot = XMMatrixRotationX((float)DEG2RAD(data.xRotation)) * XMMatrixRotationY((float)DEG2RAD(data.yRotation)) * XMMatrixRotationZ((float)DEG2RAD(data.zRotation));
		XMMATRIX mTrans = XMMatrixTranslation(data.xTranslation, data.yTranslation, data.zTranslation);
		XMMATRIX mScale = XMMatrixScaling(data.scale, data.scale, data.scale);

		// Object to world space:
		// in Order: rotation, translation, scaling
		XMMATRIX mWorld = mRot * mTrans * mScale;

		// Object to clip space
		XMMATRIX mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();

		//Inverse transposed of world for transformation of normals
		XMMATRIX worldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));

		//set effect variables
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&worldNormals));

		//render ground object
		g_Meshes.at(data.meshIdentifier)->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);
	}

	#pragma endregion

	#pragma region Render Enemy Objects

	for (list<Enemy*>::iterator it = g_EnemyInstances.begin(); it != g_EnemyInstances.end(); it++)
	{
		Enemy* enemy = *it;
		EnemyType data = *configParserInstance.getEnemyTypeByName(enemy->getEnemyType());

		XMMATRIX mRot = XMMatrixRotationX((float)DEG2RAD(data.xRotation)) * XMMatrixRotationY((float)DEG2RAD(data.yRotation)) * XMMatrixRotationZ((float)DEG2RAD(data.zRotation));
		XMMATRIX mTrans = XMMatrixTranslation(data.xTranslation, data.yTranslation, data.zTranslation);
		XMMATRIX mScale = XMMatrixScaling(data.scale, data.scale, data.scale);

		XMVECTOR velocity = XMVector3Normalize(enemy->getVel());
		XMMATRIX curRot = XMMatrixRotationY(atan2(XMVectorGetX(velocity), XMVectorGetZ(velocity)));
		XMMATRIX curTrans = XMMatrixTranslation(XMVectorGetX(enemy->getPos()), XMVectorGetY(enemy->getPos()), XMVectorGetZ(enemy->getPos()));


		//XMMATRIX t = mRot * mTrans * mScale * curRot * curTrans;

		XMMATRIX mObject = mRot * mTrans * mScale;
		XMMATRIX mAnim = curRot * curTrans;

		XMMATRIX mWorld = mObject * mAnim;
		//XMMATRIX mWorld = mAnim * mObject;

		XMMATRIX mWorldView = mWorld * g_camera.GetViewMatrix();

		// Object to clip space
		XMMATRIX mWorldViewProj = mWorld * g_camera.GetViewMatrix() * g_camera.GetProjMatrix();

		//Inverse transposed of world for transformation of normals
		XMMATRIX worldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));

		//set effect variables
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&worldNormals));


		//render ground object
		g_Meshes.at(data.meshIdentifier)->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);

	}

	#pragma endregion


	#pragma region render sprites

	vector<SpriteVertex> sprites{};

	for (list<Projectile>::iterator it = g_Projectiles.begin(); it != g_Projectiles.end(); it++)
	{
		Projectile p = *it;

		XMFLOAT3 pos = XMFLOAT3(XMVectorGetX(p.position), XMVectorGetY(p.position), XMVectorGetZ(p.position));

		SpriteVertex sv = SpriteVertex{ pos, p.origin->config->spriteRadius, p.origin == &g_GatlingGun ? 0 : 1 };
		sprites.push_back(sv);
	}

	sort(sprites.begin(), sprites.end(), [](const SpriteVertex& i, const SpriteVertex& j)
		{
			XMVECTOR v1 = XMVectorSet(i.position.x, i.position.y, i.position.z, 1);
			XMVECTOR v2 = XMVectorSet(j.position.x, j.position.y, j.position.z, 1);
			return (XMVectorGetX(XMVector3Dot(v1, g_camera.GetWorldAhead())) > XMVectorGetX(XMVector3Dot(v2, g_camera.GetWorldAhead())));
		});


	g_SpriteRenderer->renderSprites(pd3dImmediateContext, sprites, g_camera);

	#pragma endregion


	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	V(g_hud.OnRender(fElapsedTime));
	V(g_sampleUI.OnRender(fElapsedTime));
	RenderText();
	DXUT_EndPerfEvent();

	static DWORD dwTimefirst = GetTickCount();
	if (GetTickCount() - dwTimefirst > 5000)
	{
		OutputDebugString(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		OutputDebugString(L"\n");
		dwTimefirst = GetTickCount();
	}
}
