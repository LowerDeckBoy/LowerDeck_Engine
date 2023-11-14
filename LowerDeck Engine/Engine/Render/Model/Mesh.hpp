#pragma once
#include "../../Graphics/Texture.hpp"
#include "../../Graphics/Buffer/Vertex.hpp"
#include <DirectXMath.h>

namespace model 
{
	struct Material
	{
		DirectX::XMFLOAT4 BaseColorFactor{ DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		DirectX::XMFLOAT4 EmissiveFactor { DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		float MetallicFactor	{ 1.0f };
		float RoughnessFactor	{ 1.0f };
		float AlphaCutoff		{ 0.5f };
		alignas(4) bool bDoubleSided{ false };

		int32_t BaseColorIndex			{ -1 };
		int32_t NormalIndex				{ -1 };
		int32_t MetallicRoughnessIndex	{ -1 };
		int32_t EmissiveIndex			{ -1 };
	};

	// Redundant
	// Meant for pushing constants
	// -1 equals lack of index
	//struct MaterialIndices
	//{
	//	int32_t BaseColorIndex			{ -1 };
	//	int32_t NormalIndex				{ -1 };
	//	int32_t MetallicRoughnessIndex	{ -1 };
	//	int32_t EmissiveIndex			{ -1 };
	//};

	struct Mesh
	{
		std::string Name;
		DirectX::XMMATRIX Matrix{ DirectX::XMMatrixIdentity() };

		uint32_t FirstIndexLocation{ 0 };
		uint32_t BaseVertexLocation{ 0 };

		uint32_t IndexCount{ 0 };
		uint32_t VertexCount{ 0 };
		uint32_t StartVertexLocation{ 0 };
		bool bHasIndices{ false };

		//int32_t MaterialIndex{ -1 };
	};

	struct BoundingBox
	{
		DirectX::XMFLOAT3 Min;
		DirectX::XMFLOAT3 Max;
		bool bIsValid{ false };
	};

	struct Node
	{
		Node* Parent{ nullptr };
		std::vector<Node*> Children;
		std::string Name;

		DirectX::XMMATRIX Matrix{ DirectX::XMMatrixIdentity() };
		DirectX::XMFLOAT3 Translation{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
		DirectX::XMFLOAT4 Rotation{ DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		DirectX::XMFLOAT3 Scale{ DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) };
	};
}
