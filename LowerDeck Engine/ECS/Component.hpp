#pragma once
#include <DirectXMath.h>

// TODO:
namespace ecs
{
	namespace comp
	{
		
	}

	/// <summary>
	/// Transform matrix
	/// </summary>
	struct TransformComponent
	{
		DirectX::XMFLOAT4X4 Transform;
		operator const DirectX::XMFLOAT4X4& () { return Transform; }
	};

	/// <summary>
	/// XYZ 
	/// </summary>
	struct TranslationComponent
	{
		DirectX::XMFLOAT3 Translation;
		operator const DirectX::XMFLOAT3&() { return Translation; }
	};

	/// <summary>
	/// XYZW
	/// </summary>
	struct RotationComponent
	{
		DirectX::XMFLOAT4 Rotation;
		operator const DirectX::XMFLOAT4& () { return Rotation; }
	};

	/// <summary>
	/// XYZ
	/// </summary>
	struct ScaleComponent
	{
		DirectX::XMFLOAT3 Scale;
		operator const DirectX::XMFLOAT3& () { return Scale; }
	};

}
