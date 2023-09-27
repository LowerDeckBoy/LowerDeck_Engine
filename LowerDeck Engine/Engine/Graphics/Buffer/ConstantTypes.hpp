#pragma once
#include <DirectXMath.h>

namespace gfx
{
	struct cbPerObject
	{
		DirectX::XMFLOAT4X4 WVP;
		DirectX::XMFLOAT4X4 World;
	};
}
