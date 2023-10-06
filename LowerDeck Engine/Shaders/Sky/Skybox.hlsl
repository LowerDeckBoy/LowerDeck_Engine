#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

cbuffer cbPerObject : register(b0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};


VS_OUTPUT VSmain(VS_INPUT vin)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	output.Position = mul(WVP, float4(vin.Position, 1.0f)).xyzw;
	output.TexCoord = normalize(vin.Position);
    
	return output;
};

static const float2 invAtan = float2(0.1591f, 0.3183f);

cbuffer TextureIndex : register(b1, space0)
{
	int SkyboxIndex;
}

//TextureCube skyboxTexture : register(t0);
SamplerState texSampler : register(s0);

float4 PSmain(VS_OUTPUT pin) : SV_TARGET
{
	TextureCube<float4> skyboxTex = ResourceDescriptorHeap[SkyboxIndex];
	float3 skyTexture = skyboxTex.Sample(texSampler, pin.TexCoord).rgb;
	//float3 skyTexture = skyboxTexture.Sample(texSampler, pin.TexCoord).rgb;
    
	float3 gamma = float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f);
	skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
	skyTexture = pow(skyTexture, gamma);
    
	return float4(skyTexture, 1.0f);
	//return float4(1.0f, 0.5f, 1.0f, 1.0f);
}

#endif // SKYBOX_HLSL
