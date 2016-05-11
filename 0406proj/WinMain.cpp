#include "WinMain.h"
#include "MyTime.h"

int APIENTRY WinMain( HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPSTR lpszCmdParam,
					  int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize		= sizeof(WNDCLASSEX);
	wcex.style		= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon	= LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hIconSm= LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;

	if( !RegisterClassEx(&wcex) )
		return 0;

	HWND	hWnd = CreateWindowEx( WS_EX_APPWINDOW
		, szWindowClass
		, szTitle
		, WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, 800
		, 600
		, NULL
		, NULL
		, hInstance
		, NULL );

	if( !hWnd )
		return 0;

	ShowWindow(hWnd, nCmdShow);

	g_hWnd = hWnd;
	InitDevice();
	LoadTexture();
	//CreateShader();
	CreateEffectShader();
	CreateVertexBuffer();
	CreateIndexBuff();
	InitMatrix();
	CreateConstantBuffer();
	//CreateRenderState();
	//CreateDepthStencilTexture();

	CMyTime timer;
	timer.Init();

	MSG			msg;
	while( true )
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			static float deltaTime = 0;
			timer.ProcessTime();
			deltaTime += timer.GetElapsedTime();

			Render(deltaTime);
		}
	}

	CleanupDevice();
	return (int)msg.wParam;
}

// 메시지 처리 함수
LRESULT CALLBACK WndProc( HWND hWnd
						 , UINT message
						 , WPARAM wParam
						 , LPARAM lParam )
{
	HDC	hdc;
	PAINTSTRUCT	ps;

	switch(message)
	{
	case WM_CREATE:

		break;
	case WM_PAINT:
		{
			hdc = BeginPaint( hWnd, &ps );

			EndPaint( hWnd, &ps );
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	//Flag 설정
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1; //백버퍼 갯수

	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //백버퍼 포맷
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1; // sample count 기능을 쓰지 않겠다!
	sd.SampleDesc.Quality = 0; // sample 퀄리티 최하옵션
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		0,							//기본 디스플레이 어댑터 사용
		D3D_DRIVER_TYPE_HARDWARE,	//3D 하드웨어 가속
		0,							//소프트웨어 구동 안함
		createDeviceFlags,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,			// SDK 버전
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		&g_featureLevel,
		&g_pImmediateContext
		);

	if (FAILED(hr))
		return hr;

	//Create a render target view
	ID3D11Texture2D*	pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(
		0, //후면 버퍼 인덱스, 여러개일때 중요
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(
		pBackBuffer,
		NULL,
		&g_pRenderTargetView); // 이놈이 우리가 사용할 핸들

	pBackBuffer->Release(); //버퍼 사용 후에는 반환
	
	if (FAILED(hr))
		return hr;
	
	CreateDepthStencilTexture();
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		g_pDepthStencilView);

	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	return hr;
}


HRESULT LoadTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		g_pd3dDevice,
		L"Texture/images.jpg",
		NULL,
		NULL,
		&g_pTextureRV,
		NULL
		);

	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	
	if (FAILED(hr))
		return hr;
}

void Render(float deltaTime)
{
	//Clear
	float ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	g_pImmediateContext->ClearDepthStencilView(
		g_pDepthStencilView,	//clear target
		D3D11_CLEAR_DEPTH,		//Clear flag(depth, stencil)
		1.0f,					//depth buffer 지울 때 채울 값
		0						//stencil buffer 지울때 초기값
		);
	// 그리는 부분
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	//Set Shader and Draw
	//g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	//g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	
	//g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	//g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

	//rset render state
	//g_pImmediateContext->RSSetState(g_pSolidRS);
	

	CalculateMatrixForBox(deltaTime);

	D3DX11_TECHNIQUE_DESC techDesc;
	gTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		gTech->GetPassByIndex(p)->Apply(0, g_pImmediateContext);
		g_pImmediateContext->DrawIndexed(72, 0, 0);
	}
	//Render(백버퍼를 프론트 버퍼로 그린다.)
	g_pSwapChain->Present(0, 0);

}

void CreateShader()
{
	ID3DBlob *pVSBlob = NULL;
	ID3DBlob *pPSBlob = NULL;
	ID3DBlob *pErrorBlob = NULL;
	HRESULT hr;

	//vertex shader
	hr = D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		"VS", "vs_5_0",
		0, 0, 0,
		&pVSBlob, &pErrorBlob, 0);

	if (FAILED(hr))
		return;

	hr = g_pd3dDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		0, &g_pVertexShader);

	if (FAILED(hr))
		return;

	//pixel shader
	hr = D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		"PS", "ps_5_0",
		0, 0, 0,
		&pPSBlob, &pErrorBlob, 0);

	if (FAILED(hr))
		return;

	hr = g_pd3dDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		0, &g_pPixelShader);

	pPSBlob->Release();

	// Myvertex 정보
	D3D11_INPUT_ELEMENT_DESC	layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(
		layout, numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_pInputLayout);

	pVSBlob->Release();

	if (FAILED(hr))
		return;


	return;
}


void CreateEffectShader()
{
	ID3DBlob *pFxBlob = NULL;
	ID3DBlob *pErrorBlob = NULL;
	HRESULT hr;

	//vertex shader
	hr = D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		NULL , "fx_5_0",
		0, 0, 0,
		&pFxBlob, &pErrorBlob, 0);

	if (pErrorBlob != 0)
	{
		MessageBoxA(0, (char*)pErrorBlob->GetBufferPointer(), 0, 0);
		pErrorBlob->Release();
	}

	D3DX11CreateEffectFromMemory(
		pFxBlob->GetBufferPointer(),
		pFxBlob->GetBufferSize(),
		0, g_pd3dDevice, &gFX);
		
	pFxBlob->Release();
	
	gTech = gFX->GetTechniqueByName("ColorTech");
	gfxWorldViewProj = gFX->GetVariableByName("wvp")->AsMatrix();

	D3D11_INPUT_ELEMENT_DESC	layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC passDesc;
	gTech->GetPassByIndex(0)->GetDesc(&passDesc);

	UINT numElements = ARRAYSIZE(layout);
	hr = g_pd3dDevice->CreateInputLayout(
		layout, numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pInputLayout);

}


void CreateVertexBuffer()
{
	MyVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f),  },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)  },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 3.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 3.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)  },
		{ XMFLOAT3(1.0f, 3.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)   },
		{ XMFLOAT3(-1.0f, 3.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  },
		{ XMFLOAT3(-1.0f, 2.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 2.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)  },
		{ XMFLOAT3(1.0f, 2.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)   },
		{ XMFLOAT3(-1.0f, 2.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)  },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	g_pd3dDevice->CreateBuffer(
		&bd,
		&initData,
		&g_pVertexBuffer);

}


void CreateIndexBuff()
{
	UINT indices[] =
	{
		3, 1, 0,
		2, 1, 3,
		0, 5, 4,
		1, 5, 0,
		3, 4, 7,
		0, 4, 3,
		1, 6, 5,
		2, 6, 1,
		2, 7, 6,
		3, 7, 2,
		6, 4, 5,
		7, 4, 6,

		3+8, 1+8, 0+8,
		2+8, 1+8, 3+8,
		0+8, 5+8, 4+8,
		1+8, 5+8, 0+8,
		3+8, 4+8, 7+8,
		0+8, 4+8, 3+8,
		1+8, 6+8, 5+8,
		2+8, 6+8, 1+8,
		2+8, 7+8, 6+8,
		3+8, 7+8, 2+8,
		6+8, 4+8, 5+8,
		7+8, 4+8, 6+8,
	};

	D3D11_BUFFER_DESC	ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA	initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = indices;
	g_pd3dDevice->CreateBuffer(&ibd, &initData, &g_pIndexBuffer);

}

void CreateConstantBuffer()
{
	D3D11_BUFFER_DESC 	cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	g_pd3dDevice->CreateBuffer(&cbd, NULL, &g_pConstantBuffer);
}

void InitMatrix()
{
	// World 행렬 초기화
	g_World = XMMatrixIdentity();

	// View 행렬 구성
	XMVECTOR pos = XMVectorSet(-30.0f, 70.0f, 10.0f, 1.0f);
	XMVECTOR target = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
	XMVECTOR 	up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(pos, target, up);

	// Projection 행렬
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2,  	// pi
					800.0f / 600.0f,  // aspect ratio
					.01f, 1000.0f);  	// near plane, far plane

}

void CalculateMatrixForBox(float deltaTime)
{
	// 박스를 회전시키기 위한 연산. 위치, 크기를 변경하고자 한다면 SRT를 기억할 것.      
	//XMMATRIX mat  = XMMatrixRotationY(deltaTime);
	//mat *= XMMatrixRotationX(deltaTime);
	//g_World = mat;

	//XMMATRIX wvp = g_World * g_View * g_Projection;
	//ConstantBuffer cb;
	//cb.wvp = XMMatrixTranspose(wvp);

	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, 0, &cb, 0, 0); // update data
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);// set constant buffer.

	XMMATRIX mat = XMMatrixIdentity();
	mat *= XMMatrixScaling(5.0f, 5.0f, 5.0f);
	mat *= XMMatrixRotationY(deltaTime);
	mat *= XMMatrixRotationX(deltaTime);
	mat *= XMMatrixTranslation(50.0f, 10.0f, 50.0f);
	g_World = mat;

	XMMATRIX wvp = g_World * g_View * g_Projection;

	gfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&wvp));

}

void CreateDepthStencilTexture()
{
	//create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = 800;
	descDepth.Height = 600;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);

	//creste the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
}

void CreateRenderState()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;

	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &g_pSolidRS);
}
void CreateRenderStateToWire()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;

	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &g_pWireRS);
}

void CleanupDevice()
{
	if (g_pIndexBuffer)g_pIndexBuffer->Release();
	if (g_pVertexBuffer)g_pVertexBuffer->Release();
	if (g_pInputLayout)g_pInputLayout->Release();
	if (g_pVertexShader)g_pVertexShader->Release();
	if (g_pPixelShader)g_pPixelShader->Release();

	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain)g_pSwapChain->Release();
	if (g_pd3dDevice)g_pd3dDevice->Release();
	if (g_pSolidRS)g_pSolidRS->Release();

	if(g_pDepthStencil) g_pDepthStencil->Release();
	if(g_pDepthStencilView) g_pDepthStencilView->Release();
	if(g_pSolidRS )g_pSolidRS->Release();
	if (g_pWireRS)g_pWireRS->Release();
	if (g_pTextureRV)g_pTextureRV->Release();
	if (g_pSamplerLinear)g_pSamplerLinear->Release();

	if (g_pImmediateContext)g_pImmediateContext->ClearState();
}
