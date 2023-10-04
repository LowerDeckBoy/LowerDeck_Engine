#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

//#include "GBuffer.hlsli"
#include "Shaders/Deferred/GBuffer.hlsli"

// https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html#resourcedescriptorheap-and-samplerdescriptorheap
// https://rtarun9.github.io/blogs/bindless_rendering/#using-sm66s-resourcedescriptorheap--samplerdescriptorheap

cbuffer cbPerObject : register(b0, space0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
}

DeferredOutput VSmain(DeferredInput vin)
{
	DeferredOutput output = (DeferredOutput) 0;
	output.Position		 = mul(WVP, float4(vin.Position, 1.0f));
	output.WorldPosition = mul(World, float4(vin.Position, 1.0f));
	output.TexCoord		 = vin.TexCoord;
	output.Normal		 = normalize(mul((float3x3) World, vin.Normal));
	output.Tangent		 = normalize(mul((float3x3) World, vin.Tangent));
	output.Bitangent	 = normalize(mul((float3x3) World, vin.Bitangent));

	return output;
}

GBuffer_Output PSmain(DeferredOutput pin)
{
	GBuffer_Output output = (GBuffer_Output) 0;
	
	if (Material.BaseColorIndex > -1)
	{
		Texture2D<float4> baseColor = ResourceDescriptorHeap[Material.BaseColorIndex];
		output.Albedo = baseColor.Sample(texSampler, pin.TexCoord) * Material.BaseColorFactor;

		if (output.Albedo.a < Material.AlphaCutoff)
			clip(output.Albedo.a - Material.AlphaCutoff);

	}
	else
	{
		output.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	
	if (Material.NormalIndex > -1)
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
	
	if (Material.MetallicRoughnessIndex >= 0)
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
	
	if (Material.EmissiveIndex >= 0)
	{
		Texture2D<float4> emissiveTex = ResourceDescriptorHeap[Material.EmissiveIndex];
		output.Albedo += (emissiveTex.Sample(texSampler, pin.TexCoord) * Material.EmissiveFactor);
	}

	output.WorldPosition = pin.WorldPosition;
	
	const float z = pin.Position.z / pin.Position.w;
	output.DepthMap = float4(z, z, z, 1.0f);

	return output;
}

#endif // GBUFFER_HLSL
