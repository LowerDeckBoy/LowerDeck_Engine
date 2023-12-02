#define BLOCK_SIZE 8
#define GROUP_SIZE BLOCK_SIZE * BLOCK_SIZE

#define WIDTH_HEIGHT_EVEN 0
#define WIDTH_ODD_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_HEIGHT_ODD 3

struct MipGenerationData
{
	uint SrcMipIndex; // The source texture index in the descriptor heap
	uint DestMipIndex; // The destination texture index in the descriptor heap
	uint SrcMipLevel; // The level of the source mip
	uint NumMips; // Number of mips to generate in current dispatch
	uint SrcDimension; // Bitmask to specify if source mip is odd in width and/or height
	bool IsSRGB; // Is the texture in SRGB color space? Apply gamma correction
	float2 TexelSize; // 1.0 / OutMip0.Dimensions
};

ConstantBuffer<MipGenerationData> MipGenData : register(b0);

//Texture2D Texture2DTable[] : register(t0);
//RWTexture2D<float4> RWTexture2DTable[] : register(u0);

SamplerState SampLinearClamp : register(s0);

groupshared float group_R[GROUP_SIZE];
groupshared float group_G[GROUP_SIZE];
groupshared float group_B[GROUP_SIZE];
groupshared float group_A[GROUP_SIZE];

void Store(uint index, float4 color)
{
	group_R[index] = color.r;
	group_G[index] = color.g;
	group_B[index] = color.b;
	group_A[index] = color.a;
}

float4 Load(uint index)
{
	return float4(group_R[index], group_G[index], group_B[index], group_A[index]);
}

float3 ToLinear(float3 srgb)
{
	if (any(srgb < float3(0.04045f, 0.04045f, 0.04045f)))
		return srgb / float3(12.92f, 12.92f, 12.92f);
	
	return pow((srgb + float3(0.055f, 0.055f, 0.055f)) / 1.055f, 2.4f);
}

float3 ToSRGB(float3 lin)
{
	if (any(lin < 0.0031308))
		return 12.92 * lin;

	return (1.055 * pow(abs(lin), 1.0f / 2.4f) - 0.055f);
}

float4 PackColor(float4 color)
{
	if (MipGenData.IsSRGB)
		return float4(ToSRGB(color.rgb), color.a);
	else
		return color;
}

struct ComputeShaderInput
{
	uint3 GroupID : SV_GroupID;
	uint3 GroupThreadID : SV_GroupThreadID;
	uint3 DispatchThreadID : SV_DispatchThreadID;
	uint GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput IN)
{
	float4 src1 = (float4) 0;
	Texture2D<float4> srcTex = ResourceDescriptorHeap[MipGenData.SrcMipIndex];

	switch (MipGenData.SrcDimension)
	{
		case WIDTH_HEIGHT_EVEN:
	{
				float2 uv = MipGenData.TexelSize * (IN.DispatchThreadID.xy + 0.5f);
				//uv.y *= -1.0f;
				src1 = srcTex.SampleLevel(SampLinearClamp, uv, MipGenData.SrcMipLevel);
				//src1 = Texture2DTable[MipGenData.SrcMipIndex].SampleLevel(SampLinearClamp, uv, MipGenData.SrcMipLevel);
			}
			break;
		case WIDTH_ODD_HEIGHT_EVEN:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.25f, 0.5f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * float2(0.5f, 0.0f);

				src1 = 0.5f * (srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel) + srcTex.SampleLevel(SampLinearClamp, uv1 + off, MipGenData.SrcMipLevel));
			}
			break;
		case WIDTH_EVEN_HEIGHT_ODD:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.5f, 0.25f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * float2(0.0f, 0.5f);

				src1 = 0.5f * (srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel) + srcTex.SampleLevel(SampLinearClamp, uv1 + off, MipGenData.SrcMipLevel));
			}
			break;
		case WIDTH_HEIGHT_ODD:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.25f, 0.25f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * 0.5f;

				src1  = srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(off.x, 0.0f), MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(0.0f, off.y), MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(off.x, off.y), MipGenData.SrcMipLevel);
				src1 *= 0.25f;
			}
			break;
	}

	RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex];
	destTex[IN.DispatchThreadID.xy] = PackColor(src1);

	
	if (MipGenData.NumMips == 1)
		return;

	Store(IN.GroupIndex, src1);
	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x9) == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x01);
		float4 src3 = Load(IN.GroupIndex + 0x01);
		float4 src4 = Load(IN.GroupIndex + 0x01);
		src1 = 0.25f * (src1 + src2 + src3 + src4);

		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex];
		destTex[IN.DispatchThreadID.xy / 2] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}

	if (MipGenData.NumMips == 2)
		return;

	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x1B) == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x02);
		float4 src3 = Load(IN.GroupIndex + 0x10);
		float4 src4 = Load(IN.GroupIndex + 0x12);
		src1 = 0.25f * (src1 + src2 + src3 + src4);

		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex + 2];
		destTex[IN.DispatchThreadID.xy / 4] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}

	if (MipGenData.NumMips == 3)
		return;

	GroupMemoryBarrierWithGroupSync();

	if (IN.GroupIndex == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x04);
		float4 src3 = Load(IN.GroupIndex + 0x20);
		float4 src4 = Load(IN.GroupIndex + 0x24);
		src1 = 0.25f * (src1 + src2 + src3 + src4);
		
		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex + 3];
		destTex[IN.DispatchThreadID.xy / 8] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}
}

/*
#ifndef MIPMAPS_CS_HLSL
#define MIPMAPS_CS_HLSL

Texture2D<float4> SourceTex : register(t0);

RWTexture2D<float4> OutMip0 : register(u0);
RWTexture2D<float4> OutMip1 : register(u1);
RWTexture2D<float4> OutMip2 : register(u2);
RWTexture2D<float4> OutMip3 : register(u3);

SamplerState BilinearClampSampler : register(s0);

struct BindData
{
	uint mipBase;
	uint mipCount;
	float2 texelSize;
	// Boundary
	uint4 outputTextureIndices;
	// Boundary
	uint inputTextureIndex;
	//uint sRGB;
	uint resourceType; // 0=2d, 1=2d array.
	uint layer;
};

ConstantBuffer<BindData> bindData : register(b0);

//cbuffer MipData : register(b0)
//{
//	uint SrcMipLevel;	// Texture level of source mip
//	uint NumMipLevels;	// Number of OutMips to write: [1, 4]
//	float2 TexelSize;	// 1.0 / OutMip1.Dimensions
//}

// https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/GenerateMipsCS.hlsli
// https://github.com/adepke/VanguardEngine/blob/master/VanguardEngine/Shaders/Utils/Mipmap2d.hlsl

groupshared float gs_R[64];
groupshared float gs_G[64];
groupshared float gs_B[64];
groupshared float gs_A[64];

void StoreGroupColor(uint index, float4 color)
{
	gs_R[index] = color.r;
	gs_G[index] = color.g;
	gs_B[index] = color.b;
	gs_A[index] = color.a;
}

float4 LoadGroupColor(uint index)
{
	return float4(gs_R[index], gs_G[index], gs_B[index], gs_A[index]);
}

//float4 SRGBAdjustedColor(float4 sample)
//{
//	if (bindData.sRGB)
//	{
//		sample.rgb = LinearToSRGB(sample.rgb);
//	}
//	
//	return sample;
//}

void WriteMip(float4 source, uint2 xy, uint outputIndex)
{
	switch (bindData.resourceType)
	{
		case 0:
		{
				RWTexture2D<float4> mip = ResourceDescriptorHeap[bindData.outputTextureIndices[outputIndex]];
				mip[xy] = source;
				//mip[xy] = SRGBAdjustedColor(source);
				break;
			}
		case 1:
		{
				RWTexture2DArray<float4> mip = ResourceDescriptorHeap[bindData.outputTextureIndices[outputIndex]];
				//mip[uint3(xy, bindData.layer)] = SRGBAdjustedColor(source);
				mip[uint3(xy, bindData.layer)] = source;
				break;
			}
	}
}

struct Input
{
	uint groupIndex : SV_GroupIndex;
	uint3 dispatchId : SV_DispatchThreadID;
};

SamplerState bilinearClamp : register(s0);


[numthreads(8, 8, 1)]
void main(Input input)
{
	float2 offset = bindData.texelSize * 0.5;
	
	float4 sourceSample;
	
	switch (bindData.resourceType)
	{
		case 0:
		{
				Texture2D<float4> inputTexture = ResourceDescriptorHeap[bindData.inputTextureIndex];
			
				float2 uv = bindData.texelSize * (input.dispatchId.xy + float2(0.25, 0.25));
			
			// Perform 4 samples to prevent undersampling non power of two textures.
				sourceSample = inputTexture.SampleLevel(bilinearClamp, uv, bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float2(offset.x, 0.0), bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float2(0.0, offset.y), bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float2(offset.x, offset.y), bindData.mipBase);
				break;
			}
		case 1:
		{
				Texture2DArray<float4> inputTexture = ResourceDescriptorHeap[bindData.inputTextureIndex];
			
				float3 uv = float3(bindData.texelSize * (input.dispatchId.xy + float2(0.25, 0.25)), bindData.layer);

			// Perform 4 samples to prevent undersampling non power of two textures.
				sourceSample = inputTexture.SampleLevel(bilinearClamp, uv, bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float3(offset.x, 0.0, 0.0), bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float3(0.0, offset.y, 0.0), bindData.mipBase);
				sourceSample += inputTexture.SampleLevel(bilinearClamp, uv + float3(offset.x, offset.y, 0.0), bindData.mipBase);
				break;
			}
	}
	
	sourceSample *= 0.25;
	
	WriteMip(sourceSample, input.dispatchId.xy, 0);
	
	if (bindData.mipCount == 1)
	{
		return;
	}
	
	StoreGroupColor(input.groupIndex, sourceSample);
	
	GroupMemoryBarrierWithGroupSync();
	
	// X and Y are even.
	if ((input.groupIndex & 0x9) == 0)
	{
		sourceSample += LoadGroupColor(input.groupIndex + 0x01);
		sourceSample += LoadGroupColor(input.groupIndex + 0x08);
		sourceSample += LoadGroupColor(input.groupIndex + 0x09);
		
		sourceSample *= 0.25;

		WriteMip(sourceSample, input.dispatchId.xy / 2, 1);

		StoreGroupColor(input.groupIndex, sourceSample);
	}
	
	if (bindData.mipCount == 2)
	{
		return;
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	// X and Y are multiples of 4.
	if ((input.groupIndex & 0x1B) == 0)
	{
		sourceSample += LoadGroupColor(input.groupIndex + 0x02);
		sourceSample += LoadGroupColor(input.groupIndex + 0x10);
		sourceSample += LoadGroupColor(input.groupIndex + 0x12);
		
		sourceSample *= 0.25;

		WriteMip(sourceSample, input.dispatchId.xy / 4, 2);
		
		StoreGroupColor(input.groupIndex, sourceSample);
	}
	
	if (bindData.mipCount == 3)
	{
		return;
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	// X and Y are multiples of 8 (only one thread qualifies).
	if (input.groupIndex == 0)
	{
		sourceSample += LoadGroupColor(input.groupIndex + 0x04);
		sourceSample += LoadGroupColor(input.groupIndex + 0x20);
		sourceSample += LoadGroupColor(input.groupIndex + 0x24);
		
		sourceSample *= 0.25;
		
		WriteMip(sourceSample, input.dispatchId.xy / 8, 3);
	}
}

#endif // MIPMAPS_CS_HLSL
*/