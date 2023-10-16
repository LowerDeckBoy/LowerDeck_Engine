#ifndef GBUFFER_HLSLI
#define GBUFFER_HLSLI

#define INVALID_INDEX -1

struct DeferredOutput
{
	float4 Position			: SV_POSITION;
	float4 WorldPosition	: WORLD_POSITION;
	float2 TexCoord			: TEXCOORD;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitangent		: BITANGENT;
};

struct GBufferOutput
{
	float4 DepthMap			: SV_Target0;
	float4 Albedo			: SV_Target1;
	float4 Normal			: SV_Target2;
	float4 Metallic			: SV_Target3;
	float4 WorldPosition	: SV_Target4;
};

// Vertex Buffer layout
struct VertexLayout
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

#endif // GBUFFER_HLSLI
