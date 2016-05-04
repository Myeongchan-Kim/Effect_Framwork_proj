#pragma once
#include <windows.h>
#include <time.h>
#include <DXGI.h>
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <D3DX11.h>
#include <DxErr.h>
#include "MyVertex.h"

#define szWindowClass	TEXT("First")
#define szTitle			TEXT("First App")

HWND					g_hWnd = NULL;
IDXGISwapChain*			g_pSwapChain = NULL;
ID3D11Device*			g_pd3dDevice = NULL;
ID3D11DeviceContext*	g_pImmediateContext = NULL;
ID3D11RenderTargetView*	g_pRenderTargetView = NULL;
ID3D11VertexShader*		g_pVertexShader = NULL;
ID3D11InputLayout*		g_pVertexLayout = NULL;
ID3D11Buffer*			g_pVertexBuffer = NULL;
ID3D11PixelShader*		g_pPixelShader = NULL;
ID3D11Buffer*			g_pIndexBuffer = NULL;
ID3D11Buffer*			g_pConstantBuffer;
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;

ID3D11Texture2D*		g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11RasterizerState*	g_pSolidRS;
ID3D11RasterizerState*	g_pWireRS;

ID3D11ShaderResourceView* g_pTextureRV = NULL;
ID3D11SamplerState*		g_pSamplerLinear = NULL;

D3D_FEATURE_LEVEL		g_featureLevel = D3D_FEATURE_LEVEL_11_0;

LRESULT CALLBACK WndProc(HWND hWnd
	, UINT message
	, WPARAM wParam
	, LPARAM lParam);

HRESULT InitDevice();
HRESULT LoadTexture();

void Render(float deltaTime);
void CleanupDevice();
void CreateShader();
void CreateVertexBuffer();
void CreateIndexBuff();
void CreateConstantBuffer();
void CalculateMatrixForBox(float deltaTime);
void InitMatrix();
void CreateDepthStencilTexture();
void CreateRenderState();
void CreateRenderStateToWire();

XMFLOAT4 lightDirection =
{
	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
};

XMFLOAT4 lightColor =
{
	XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)
};

struct ConstantBuffer
{
	XMMATRIX wvp;
	XMMATRIX world;
	XMFLOAT4 lightDir;
	XMFLOAT4 lightColor;
};
