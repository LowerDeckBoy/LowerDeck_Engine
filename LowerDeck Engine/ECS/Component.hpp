#pragma once
#include <DirectXMath.h>
#include <array>
#include <string>

// TODO:
namespace ecs
{
	namespace comp
	{
		
	}

	struct TagComponent
	{
		std::string Name{ "Empty" };
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& Tag) : Name(Tag) {}
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(DirectX::XMFLOAT3 Position)
			: Translation(Position)
		{
		}

		DirectX::XMFLOAT3 Translation	{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
		DirectX::XMFLOAT3 Rotation		{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
		DirectX::XMFLOAT3 Scale			{ DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) };

		DirectX::XMFLOAT3& GetTranslation()
		{
			return Translation;
		}
	};

	struct PositionComponent
	{
		PositionComponent() = default;
		PositionComponent(DirectX::XMFLOAT3 Position) 
			: Position(Position) { }
		
		DirectX::XMFLOAT3 Position{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
	};

	struct ColorComponent
	{
		//DirectX::XMFLOAT4 Color{ DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		std::array<float, 4> Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		
	};

	struct DirectionalLightComponent
	{
		DirectX::XMFLOAT3 Direction;
		std::array<float, 4> Color{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct PointLightComponent
	{
		DirectX::XMFLOAT4 Position;
		//DirectX::XMFLOAT4 Color{ DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		std::array<float, 4> Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Radius{ 25.0f };
	};

}
