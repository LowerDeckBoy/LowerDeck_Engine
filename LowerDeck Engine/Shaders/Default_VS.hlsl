#ifndef DEFAULT_VS_HLSL
#define DEFAULT_VS_HLSL


cbuffer cbPerObject : register(b0, space0)
{
	row_major matrix WVP;
	row_major matrix World;
}

struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal   : NORMAL;
	float3 Tangent  : TANGENT;
	float3 Bitangent  : BITANGENT;
};

struct VS_OUTPUT
{
	float4 Position	: SV_POSITION;
	float2 TexCoord	: TEXCOORD;
	float3 Normal	: NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

VS_OUTPUT main(VS_INPUT vin)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	output.Position = mul(WVP, float4(vin.Position, 1.0f));
	output.TexCoord = vin.TexCoord;
	output.Normal = normalize(mul((float3x3) World, vin.Normal));
	output.Tangent = normalize(mul((float3x3) World, vin.Tangent));
	output.Bitangent = normalize(mul((float3x3) World, vin.Bitangent));
	
	return output;
}

#endif // DEFAULT_VS_HLSL
