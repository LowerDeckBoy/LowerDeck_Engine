#ifndef DEFAULT_PS_HLSL
#define DEFAULT_PS_HLSL

struct PS_INPUT
{
	float4 Position  : SV_POSITION;
	float2 TexCoord  : TEXCOORD;
	float3 Normal	 : NORMAL;
	float3 Tangent	 : TANGENT;
	float3 Bitangent : BITANGENT;
};

float4 main(PS_INPUT pin) : SV_TARGET
{
	return float4(pin.Normal.xyz, 1.0f);
}

#endif // DEFAULT_PS_HLSL
