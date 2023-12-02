#ifndef DEFERRED_COMMON_HLSLI
#define DEFERRED_COMMON_HLSLI

struct ScreenQuadInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct ScreenQuadOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

#endif // DEFERRED_COMMON_HLSLI
