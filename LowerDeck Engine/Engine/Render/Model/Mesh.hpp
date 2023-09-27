#pragma once
//#include "../../Graphics/Texture.hpp"
#include "../../Graphics/Buffer/Vertex.hpp"
#include <DirectXMath.h>
using namespace DirectX;

namespace model 
{
	struct Material
	{
		XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		XMFLOAT4 EmissiveFactor	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		float MetallicFactor	{ 1.0f };
		float RoughnessFactor	{ 1.0f };
		float AlphaCutoff		{ 0.5f };
		alignas(4) bool bDoubleSided{ false };

		//Texture* BaseColorTexture			{ nullptr };
		//Texture* NormalTexture				{ nullptr };
		//Texture* MetallicRoughnessTexture	{ nullptr };
		//Texture* EmissiveTexture			{ nullptr };
	};

	struct MaterialData
	{
		XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		XMFLOAT4 EmissiveFactor { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		float MetallicFactor { 1.0f };
		float RoughnessFactor{ 1.0f };
		float AlphaCutoff	 { 0.5f };
		alignas(4) bool bDoubleSided{ false };

		int32_t BaseColorIndex			{ -1 };
		int32_t NormalIndex				{ -1 };
		int32_t MetallicRoughnessIndex	{ -1 };
		int32_t EmissiveIndex			{ -1 };
	};

	// Meant for pushing constants
	// -1 equals lack of index
	struct MaterialIndices
	{
		int32_t BaseColorIndex			{ -1 };
		int32_t NormalIndex				{ -1 };
		int32_t MetallicRoughnessIndex	{ -1 };
		int32_t EmissiveIndex			{ -1 };
	};

	struct Mesh
	{
		std::string Name;
		XMMATRIX Matrix{ XMMatrixIdentity() };

		uint32_t FirstIndexLocation{ 0 };
		uint32_t BaseVertexLocation{ 0 };

		uint32_t IndexCount{ 0 };
		uint32_t VertexCount{ 0 };
		uint32_t StartVertexLocation{ 0 };
		bool bHasIndices{ false };

	};

	struct Node
	{
		Node* Parent{ nullptr };
		std::vector<Node*> Children;
		std::string Name;

		XMMATRIX Matrix{ XMMatrixIdentity() };
		XMFLOAT3 Translation{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
		XMFLOAT4 Rotation{ XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		XMFLOAT3 Scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	};
}