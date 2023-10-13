#ifndef GBUFFER_HLSLI
#define GBUFFER_HLSLI

struct DeferredInput
{
	float3 Position		: POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
};

struct DeferredOutput
{
	float4 Position			: SV_POSITION;
	float4 WorldPosition	: WORLD_POSITION;
	float2 TexCoord			: TEXCOORD;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitangent		: BITANGENT;
};

struct GBuffer_Output
{
	float4 DepthMap			: SV_Target0;
	float4 Albedo			: SV_Target1;
	float4 Normal			: SV_Target2;
	float4 Metallic			: SV_Target3;
	float4 WorldPosition	: SV_Target4;
};

// Vertex Buffers
struct VertexBuffer
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

struct Vertex
{
	uint VertexIndex;
	uint VertexOffset;
};

// Textures 
struct MaterialIndices
{
	int BaseColorIndex;
	int NormalIndex;
	int MetallicRoughnessIndex;
	int EmissiveIndex;
};

struct MaterialData
{
	float4 BaseColorFactor;
	float4 EmissiveFactor;
	
	float MetallicFactor;
	float RoughnessFactor;
	float AlphaCutoff;
	bool bDoubleSided;

	int BaseColorIndex;
	int NormalIndex;
	int MetallicRoughnessIndex;
	int EmissiveIndex;
};

//Texture2D<float4> DepthTexture : register(t4, space0);

#endif // GBUFFER_HLSLI
