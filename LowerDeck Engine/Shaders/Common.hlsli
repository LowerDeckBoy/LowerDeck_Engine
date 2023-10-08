#ifndef COMMON_HLSLI
#define COMMON_HLSLI

struct VertexBuffer
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

// https://alextardif.com/Bindless.html
struct Material
{
	uint VertexBufferIndex;
	uint VertexOffset;
};



#endif // COMMON_HLSLI
