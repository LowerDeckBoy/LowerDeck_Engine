#pragma once
#include <DirectXMath.h>

namespace gfx
{
	/// <summary>
	/// Object matrices.
	/// </summary>
	struct cbPerObject
	{
		DirectX::XMMATRIX WVP;
		DirectX::XMMATRIX World;
	};

	/// <summary>
	/// Object data need to draw.
	/// </summary>
	struct cbPerDraw
	{
		int32_t MeshIndex;
		int32_t MaterialIndex;
	};

	//XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	//XMFLOAT4 EmissiveFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	//
	//float MetallicFactor{ 1.0f };
	//float RoughnessFactor{ 1.0f };
	//float AlphaCutoff{ 0.5f };
	//alignas(4) bool bDoubleSided{ false };

	struct cbMaterialIndices
	{
		int32_t BaseColorIndex { - 1 };
		int32_t NormalIndex { - 1 };
		int32_t MetalRoughnessIndex { - 1 };
		int32_t EmissiveIndex { - 1 };
	};
}
