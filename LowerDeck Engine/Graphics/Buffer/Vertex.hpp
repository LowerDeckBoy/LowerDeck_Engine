#pragma once
#include <DirectXMath.h>

namespace gfx
{
	/// <summary>
	/// Full Vertex type.<br/>
	/// </summary>
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Texcoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 Bitangent;
	};

	/// <summary>
	/// Plain Skybox and Image Based Lighting usage.
	/// </summary>
	struct SkyVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
	};

	/// <summary>
	/// Deferred Output.
	/// </summary>
	struct ScreenOutputVertex
	{
		DirectX::XMFLOAT3 Positon;
		DirectX::XMFLOAT2 TexCoord;
	};
}
