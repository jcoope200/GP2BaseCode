float4 objectColour
<
	string UIName="Object Colour";
	string UIWidget="Color";
> ={1,1,1,1};
float4x4 matWorld:WORLD;
float4x4 matView:VIEW;
float4x4 matProjection:PROJECTION;

struct VS_INPUT
{
	float2 texCoord:TEXCOORD0;
	float4 pos:POSITION;
};

struct PS_INPUT
{
	float2 texCoord:TEXCOORD0;
	float4 pos:SV_POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output=(PS_INPUT)0;
	output.texCoord=input.texCoord;
	float4x4 matViewProjection=mul(matView,matProjection);
	float4x4 matWorldViewProjection=mul(matWorld,matViewProjection);
	
	output.pos=mul(input.pos,matWorldViewProjection);
	return output;
}
Texture2D diffuseMap;
SamplerState WrapPointSampler
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};
float4 PS(PS_INPUT input):SV_TARGET
{	
	return diffuseMap.Sample(WrapPointSampler,input.texCoord);
	return float4(objectColour);
}

RasterizerState DisableCulling
{
	CullMode = NONE;
};

technique10 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,VS()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader( ps_4_0, PS()));
		SetRasterizerState(DisableCulling);
	}
}
