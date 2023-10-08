#include "../Camera.hpp"
#include "Model.hpp"
#include "../../D3D/D3D12Device.hpp"
#include "../../D3D/D3D12Command.hpp"
#include "../../Graphics/Buffer/BufferUtility.hpp"
#include <ImGui/imgui.h>


Model::Model(std::string_view Filepath, const std::string& ModelName) 
	: Importer(Filepath)
{
	Create(Filepath);
	m_ModelName = ModelName;
}

Model::~Model()
{
	Release();
}

void Model::Create(std::string_view Filepath)
{
	if (!Import(Filepath)) 
	{
		throw std::exception();
	}

	m_VertexBuffer	= std::make_unique<gfx::VertexBuffer>(gfx::BufferData(m_Vertices.data(), m_Vertices.size(), sizeof(m_Vertices.at(0)) * m_Vertices.size(), sizeof(m_Vertices.at(0))));
	m_IndexBuffer	= std::make_unique<gfx::IndexBuffer>(gfx::BufferData(m_Indices.data(), m_Indices.size(), sizeof(uint32_t) * m_Indices.size(), sizeof(uint32_t)));

	m_cbPerObject	= std::make_unique<gfx::ConstantBuffer<gfx::cbPerObject>>(&m_cbPerObjectData);
	//m_cbMaterial	= std::make_unique<gfx::ConstantBuffer<gfx::cbMaterial>>(&m_cbMaterialData);
	//m_cbPerObject	= std::make_unique<ConstantBuffer<cbPerObject>>(pDevice, &m_cbPerObjectData);
	//m_cbCamera		= std::make_unique<ConstantBuffer<cbCamera>>(pDevice, &m_cbCameraData);

	m_Vertices.clear();
	m_Vertices.shrink_to_fit();
	m_Indices.clear();
	m_Indices.shrink_to_fit();

	UpdateWorld();
}

void Model::Draw(Camera* pCamera)
{
	D3D::g_CommandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D::g_CommandList.Get()->IASetVertexBuffers(0, 1, &m_VertexBuffer->View);

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{

		m_cbPerObject->Update({ DirectX::XMMatrixTranspose(m_Meshes.at(i)->Matrix * m_WorldMatrix * pCamera->GetViewProjection()), DirectX::XMMatrixTranspose(m_WorldMatrix) });
		D3D::g_CommandList.Get()->SetGraphicsRootConstantBufferView(0, m_cbPerObject->GetBuffer()->GetGPUVirtualAddress());
		
		auto currentMaterial{ m_Materials.at(i) };

		//XMFLOAT4 cameraPosition{};
		//XMStoreFloat4(&cameraPosition, pCamera->GetPosition());
		//cameraPosition,
		// Material data
		const model::Material materialData{  currentMaterial->BaseColorFactor,
											 currentMaterial->EmissiveFactor,
											 currentMaterial->MetallicFactor,
											 currentMaterial->RoughnessFactor,
											 currentMaterial->AlphaCutoff,
											 currentMaterial->bDoubleSided,
											 // Indices
											 currentMaterial->BaseColorIndex,
											 currentMaterial->NormalIndex,
											 currentMaterial->MetallicRoughnessIndex,
											 currentMaterial->EmissiveIndex };

		D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstants(1, sizeof(materialData) / sizeof(int32_t), &materialData, 0);
		//D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstants(2, sizeof(indices) / sizeof(int32_t), &indices, 0);

		if (m_Meshes.at(i)->bHasIndices)
		{
			D3D::g_CommandList.Get()->IASetIndexBuffer(&m_IndexBuffer->View);
			D3D::g_CommandList.Get()->DrawIndexedInstanced(m_Meshes.at(i)->IndexCount, 1,
				m_Meshes.at(i)->FirstIndexLocation,
				m_Meshes.at(i)->BaseVertexLocation, 0);
		}
		else
		{
			D3D::g_CommandList.Get()->DrawInstanced(m_Meshes.at(i)->VertexCount, 1, m_Meshes.at(i)->StartVertexLocation, 0);
		}
	}

	//DrawGUI();
}

void Model::DrawGUI()
{
	ImGui::Begin(m_ModelName.c_str());
	// Transforms
	{
		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation = XMVectorSet(m_Translations.at(0), m_Translations.at(1), m_Translations.at(2), 0.0f);
			UpdateWorld();
		}

		if (ImGui::DragFloat3("Rotation", m_Rotations.data(), 0.5f))
		{
			m_Rotation = XMVectorSet(XMConvertToRadians(m_Rotations.at(0)),
									 XMConvertToRadians(m_Rotations.at(1)),
									 XMConvertToRadians(m_Rotations.at(2)),
									 0.0f);
			UpdateWorld();
		}

		if (ImGui::DragFloat3("Scale", m_Scales.data(), 0.1f))
		{
			m_Scale = XMVectorSet(m_Scales.at(0), m_Scales.at(1), m_Scales.at(2), 0.0f);
			UpdateWorld();
		}

		if (ImGui::Button("Reset"))
		{
			m_Translations	= { 0.0f, 0.0f, 0.0f };
			m_Rotations		= { 0.0f, 0.0f, 0.0f };
			m_Scales		= { 1.0f, 1.0f, 1.0f };

			m_Translation	= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Rotation		= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Scale			= XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
			UpdateWorld();
		}
	}
	
	ImGui::End();
}

void Model::Release()
{
	m_cbPerObject.reset();
	m_VertexBuffer.reset();
	m_IndexBuffer.reset();

}

void Model::UpdateWorld() noexcept
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
