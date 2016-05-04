#include <windows.h>
#include <time.h>
#include <DXGI.h>
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <D3DX11.h>
#include <DxErr.h>

#define szWindowClass	TEXT("First")
#define szTitle			TEXT("First App")

HWND					g_hWnd = NULL;
IDXGISwapChain*			g_pSwapChain = NULL;
ID3D11Device*			g_pd3dDevice = NULL;
ID3D11DeviceContext*	g_pImmediateContext = NULL;
ID3D11RenderTargetView*	g_pRenderTargetView = NULL;

D3D_FEATURE_LEVEL		g_featureLevel = D3D_FEATURE_LEVEL_11_0;

LRESULT CALLBACK WndProc(HWND hWnd
	, UINT message
	, WPARAM wParam
	, LPARAM lParam);

HRESULT InitDevice();

void Render();
void CleanupDevice();