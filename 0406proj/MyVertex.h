#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct MyVertex
{
public:
	XMFLOAT3 pos;
	XMFLOAT4 color;

	XMFLOAT3 normal;
	XMFLOAT2 tex;
};

