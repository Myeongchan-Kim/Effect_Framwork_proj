
//Texture2D texDiffuse;
//SamplerState samLinear;

cbuffer ConstantBuffer
{
	float4x4 	wvp;
}

struct VertexIn
{
	float3 pos:POSITION;
	float4 color:COLOR;
};

struct VertexOut
{
	float4 pos:SV_POSITION;
	float4 color:COLOR;
};


VertexOut    VS(VertexIn vIn)
{
	VertexOut vOut;

	vOut.pos = mul(float4(vIn.pos, 1.0f), wvp);
	vOut.color = vIn.color;

	return vOut;
};

float4 PS(VertexOut vOut): SV_TARGET
{
	
	return vOut.color;
};

RasterizerState WireframeRS
{
	FillMode = Wireframe;
	CullMode = Back;
	FrontCounterClockwise = false;
	//설정하지 않으면 디폴트 값이 들어감.
};

technique11 ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));

		//SetRasterizerState(WireframeRS);
	}
}