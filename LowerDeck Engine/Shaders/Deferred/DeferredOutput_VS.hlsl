#ifndef DEFERRED_OUTPUT_VS_HLSL
#define DEFERRED_OUTPUT_VS_HLSL

#include "Deferred_Common.hlsli"

ScreenQuadOutput main(ScreenQuadInput vin)
{
	ScreenQuadOutput output = (ScreenQuadOutput) 0;

	output.Position = float4(vin.Position.xyz, 1.0f);
	output.TexCoord = vin.TexCoord;

	return output;
}

#endif // DEFERRED_OUTPUT_VS_HLSL
