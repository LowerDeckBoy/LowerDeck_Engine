#ifndef DEFERRED_OUTPUT_HLSL
#define DEFERRED_OUTPUT_HLSL

// Set max light count and push current light count with PushConstants
// so shader can iterate properly
//#define MAX_POINT_LIGHT_COUNT 256

#include "DeferredCommon.hlsli"
#include "DeferredLighting.hlsli"

cbuffer cbCamera : register(b0, space0)
{
	float4 CameraPosition;
	float4x4 View;
	float4x4 Projection;
	row_major float4x4 InversedView;
	row_major float4x4 InversedProjection;
	float zNear;
	float zFar;
}

ScreenQuadOutput VSmain(ScreenQuadInput vin)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;

	output.Position = float4(vin.Position.xyz, 1.0f);
	output.TexCoord = vin.TexCoord;

	return output;
}

cbuffer cbGBufferIndices : register(b1, space0)
{
	int DepthIndex;
	int BaseColorIndex;
	int NormalIndex;
	int MetalRoughnessIndex;	
	int WorldPositionIndex;
};

cbuffer cbIBLIndices : register(b2, space0)
{
	int SkyboxIndex;
	int IrradianceIndex;
	int SpecularIndex;
	int SpecularBRDFIndex;
}

cbuffer cbPointLights : register(b0, space1)
{
	float4 LightPosition[4];
	float4 LightColor[4];
}

struct OutputData
{
	float4 OutputTexture : SV_Target;
};

SamplerState texSampler : register(s0, space0);
SamplerState spBRDFSampler : register(s1, space0);

OutputData PSmain(ScreenQuadOutput pin)
{
	OutputData outputData = (OutputData) 0;

	float2 position = pin.Position.xy;
	// All textures that come from GBuffer
	Texture2D<float4> gbufferDepth		= ResourceDescriptorHeap[DepthIndex];
	Texture2D<float4> gbufferPositions	= ResourceDescriptorHeap[WorldPositionIndex];
	Texture2D<float4> gbufferBaseColor	= ResourceDescriptorHeap[BaseColorIndex];
	Texture2D<float4> gbufferNormal		= ResourceDescriptorHeap[NormalIndex];
	Texture2D<float4> gbufferMetallic	= ResourceDescriptorHeap[MetalRoughnessIndex];

	float depth = gbufferDepth.Load(int3(position, 0)).r;
	// pow 2.2 for gamma correction
	float4 baseColor	= pow(gbufferBaseColor.Load(int3(position, 0)), 2.2f);
	//float4 baseColor	= gbufferBaseColor.Load(int3(position, 0));
	float4 normal		= gbufferNormal.Load(int3(position, 0));
	float metalness		= gbufferMetallic.Load(int3(position, 0)).b;
	float roughness		= gbufferMetallic.Load(int3(position, 0)).g;
	float4 positions	= gbufferPositions.Load(int3(position, 0));

	float3 N = normal.rgb;
	float3 V = normalize(CameraPosition.xyz - positions.rgb);
	
	float NdotV = saturate(max(dot(N, V), Epsilon));
    
	//float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metalness);
	float3 F0 = lerp(0.04f, baseColor.rgb, metalness);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	// Reflection vector
	float3 Lr = reflect(-V, N);
	
	for (int i = 0; i < 4; ++i)
	{
		float3 L = normalize(LightPosition[i].xyz - positions.xyz);
		float3 H = normalize(V + L);

		float distance = length(LightPosition[i].xyz - positions.xyz);
		float attenuation = 1.0f / (distance * distance);
		float3 radiance = LightColor[i].rgb * (attenuation * LightPosition[i].w);
        
		float NdotH = max(dot(N, H), 0.0f);
		float NdotL = max(dot(N, L), 0.0f);

        // Cook-Torrance BRDF
		float NDF = GetDistributionGGX(N, H, roughness);
		float G = GetGeometrySmith(N, V, L, roughness);
		float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

		float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), metalness);
		//kD *= (1.0f - metalness);
        
		float3 numerator = NDF * G * F;
		float denominator = 4.0f * NdotV * NdotL;
		float3 specular = numerator / max(denominator, Epsilon);
        
		Lo += ((kD * baseColor.rgb / PI) + specular) * radiance * NdotL;
	}
	
	float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
		
	float3 output = ambient + Lo;
	
	float3 ambientLighting = float3(0.0f, 0.0f, 0.0f);
	{
		float3 F = GetFresnelSchlick(NdotV, F0);
		float3 kS = F;
		float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalness);
		//kD *= (1.0f - metalness);
        
		TextureCube irradianceTexture	= ResourceDescriptorHeap[IrradianceIndex];
		TextureCube specularTexture		= ResourceDescriptorHeap[SpecularIndex];
		Texture2D   specularBRDFTexture	= ResourceDescriptorHeap[SpecularBRDFIndex];
		
		float3 irradiance = irradianceTexture.Sample(texSampler, N).rgb;
		float3 diffuseIBL = (kD * baseColor.rgb * irradiance);
		
		uint width, height, levels;
		specularTexture.GetDimensions(0, width, height, levels);
		// * levels
		float3 specular = specularTexture.SampleLevel(texSampler, Lr, roughness).rgb;
		float2 specularBRDF = specularBRDFTexture.Sample(spBRDFSampler, float2(NdotV, roughness)).rg;
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specular;

		ambientLighting = (diffuseIBL + specularIBL);
	}
	
	output = output / (output + float3(1.0f, 1.0f, 1.0f));
	output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
	outputData.OutputTexture = float4(output + ambientLighting, 1.0f);
	
	return outputData;
}

#endif // DEFERRED_OUTPUTx_HLSL
