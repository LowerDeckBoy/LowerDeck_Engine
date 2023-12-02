#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "GBuffer.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
}

ConstantBuffer<MaterialData> Material	: register(b1, space0);
ConstantBuffer<Vertex> cbVertex			: register(b2, space0);

DeferredOutput VSmain(uint VertexID : SV_VertexID)
{
	// Load buffer from it's index in descriptor
	StructuredBuffer<VertexLayout> vertexBuffer = ResourceDescriptorHeap[cbVertex.VertexIndex];
	// Load current vertex from buffer
	VertexLayout vertex = vertexBuffer.Load((cbVertex.VertexOffset + VertexID));

	DeferredOutput output = (DeferredOutput) 0;
	output.Position		  = mul(WVP, float4(vertex.Position, 1.0f));
	output.WorldPosition  = mul(World, float4(vertex.Position, 1.0f));
	output.TexCoord		  = vertex.TexCoord;
	output.Normal		  = normalize(mul((float3x3) World, vertex.Normal));
	output.Tangent		  = normalize(mul((float3x3) World, vertex.Tangent));
	output.Bitangent	  = normalize(mul((float3x3) World, vertex.Bitangent));
	
	return output;
}

SamplerState texSampler : register(s0, space0);

GBufferOutput PSmain(DeferredOutput pin)
{
	GBufferOutput output = (GBufferOutput) 0;
	
	// Load BaseColor texture
	if (Material.BaseColorIndex > INVALID_INDEX)
	{
		Texture2D<float4> baseColor = ResourceDescriptorHeap[Material.BaseColorIndex];
		output.Albedo = baseColor.Sample(texSampler, pin.TexCoord) * Material.BaseColorFactor;

		// Check alpha and cut if necessary
		if (output.Albedo.a < Material.AlphaCutoff)
			clip(output.Albedo.a - Material.AlphaCutoff);
	}
	else
	{
		output.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	
	// Load and transform Normal texture
	if (Material.NormalIndex > INVALID_INDEX)
	{
		Texture2D<float4> normalTexture = ResourceDescriptorHeap[Material.NormalIndex];
		float4 normalMap = normalize(2.0f * normalTexture.Sample(texSampler, pin.TexCoord) - 1.0f);
		float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
		float3 bitangent = cross(pin.Normal, tangent);
		float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
		//normalMap.w
		output.Normal = float4(normalize(mul(normalMap.xyz, texSpace)), normalMap.w);
	}
	else
	{
		output.Normal = float4(pin.Normal, 1.0f);
	}
	
	// Load MetalRoughness texture
	if (Material.MetallicRoughnessIndex > INVALID_INDEX)
	{
		Texture2D<float4> metalRoughnessTex = ResourceDescriptorHeap[Material.MetallicRoughnessIndex];
		float4 metallic = metalRoughnessTex.Sample(texSampler, pin.TexCoord);
		metallic.b *= Material.MetallicFactor;
		metallic.g *= Material.RoughnessFactor;
		output.Metallic = metallic;
	}
	else
	{
		output.Metallic = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	
	// Load Emissive texture.
	// If available: add it's output right to BaseColor
	if (Material.EmissiveIndex > INVALID_INDEX)
	{
		Texture2D<float4> emissiveTex = ResourceDescriptorHeap[Material.EmissiveIndex];
		output.Albedo += (emissiveTex.Sample(texSampler, pin.TexCoord) * Material.EmissiveFactor);
	}

	// Depth Buffer and WorlPositions
	{
		const float z = pin.Position.z / pin.Position.w;
		output.DepthMap = float4(z, z, z, 1.0f);
		
		output.WorldPosition = pin.WorldPosition;
	}


	return output;
}

#endif // GBUFFER_HLSL
