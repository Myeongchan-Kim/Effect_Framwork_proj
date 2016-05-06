
Texture2D texDiffuse;
SamplerState samLinear;

struct VertexIn
{
	float3 pos:POSITION;
	float4 color:COLOR;

	float3 normal:NORMAL;
	float2 tex: TEXCOORD;
};

struct VertexOut
{
	float4 pos:SV_POSITION;
	float4 color:COLOR;

	float4 normal:NORMAL;
	float2 tex: TEXCOORD;
};

cbuffer ConstantBuffer
{
	float4x4 	wvp;
	float4x4	world;

	float4		lightDir;
	float4		lightColor;
}

VertexOut    VS(VertexIn vIn)
{
	VertexOut vOut;

	vOut.pos = mul(float4(vIn.pos, 1.0f), wvp);
	vOut.color = vIn.color;
	vOut.normal = mul(float4(vIn.normal, 0.0f), world);
	vOut.tex = vIn.tex;

	return vOut;
};

float4 PS(VertexOut vOut): SV_TARGET
{
	float4 finalColor = 0;

	finalColor = saturate((dot( -lightDir, vOut.normal) * 0.5f + 0.5f)) * lightColor;
	float4 texColor = texDiffuse.Sample(samLinear, vOut.tex) * finalColor;
	texColor.a = 1.0f;

	return texColor;
};


technique11 ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};