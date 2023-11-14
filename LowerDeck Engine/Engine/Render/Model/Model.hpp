#pragma once
#include "Importer.hpp"
#include "../../Graphics/Buffer/Buffer.hpp"
#include "../../Graphics/Buffer/ConstantBuffer.hpp"
#include <memory>
#include <array>

using namespace DirectX;

class Camera;
class TextureManager;

class Model : public Importer
{
public:
	Model(std::string_view Filepath, const std::string& ModelName = "Unnamed");
	//Model(std::string_view Filepath, const std::string& ModelName = "Unnamed");
	~Model();

	//void Create(std::string_view Filepath);
	void Create(std::string_view Filepath);
	void Draw(Camera* pCamera);

	void DrawGUI();

	void Release();

	// Bindless buffer
	std::unique_ptr<gfx::StructuredBuffer> m_VertexBuffer;
	std::unique_ptr<gfx::IndexBuffer> m_IndexBuffer;

protected:
	// Constant Buffers
	std::unique_ptr<gfx::ConstantBuffer<gfx::cbPerObject>> m_cbPerObject;
	gfx::cbPerObject m_cbPerObjectData{};

	// Transforms
	void UpdateWorld() noexcept;

	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Rotation		{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0) };

	// For GUI usage
	std::array<float, 3> m_Translations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Rotations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales		{ 1.0f, 1.0f, 1.0f };

};
