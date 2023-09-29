#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "GBuffer.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
}

DeferredOutput VSmain(DeferredInput vin)
{
	DeferredOutput output = (DeferredOutput) 0;
	output.Position = mul(WVP, float4(vin.Position, 1.0f));
	output.WorldPosition = mul(World, float4(vin.Position, 1.0f));
	output.TexCoord = vin.TexCoord;
	output.Normal = normalize(mul((float3x3) World, vin.Normal));
	output.Tangent = normalize(mul((float3x3) World, vin.Tangent));
	output.Bitangent = normalize(mul((float3x3) World, vin.Bitangent));

	return output;
}

//cbuffer cbMaterial : register(b0, space1)
//{
//	float3 CameraPos;
//	float padding3;
//
//	float4 BaseColorFactor;
//	float4 EmissiveFactor;
//
//	float MetallicFactor;
//	float RoughnessFactor;
//	float AlphaCutoff;
//	bool bDoubleSided;
//
//};

GBuffer_Output PSmain(DeferredOutput pin)
{
	GBuffer_Output output = (GBuffer_Output) 0;
	output.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
	output.Normal = float4(pin.Normal, 1.0f);

    
	//if (Indices.MetallicRoughnessIndex >= 0)
	//{
	//	float4 metallic = bindless_textures[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord);
	//	metallic.b *= MetallicFactor;
	//	metallic.g *= RoughnessFactor;
	//	output.Metallic = metallic;
	//}
	//else
	//{
	//	output.Metallic = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//}

	output.WorldPosition = pin.WorldPosition;
    
	float z = pin.Position.z / pin.Position.w;
	output.DepthMap = float4(z, z, z, 1.0f);

	return output;
}


#endif // GBUFFER_HLSL
