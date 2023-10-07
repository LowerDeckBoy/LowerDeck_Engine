#include "PointLights.hpp"
#include <ImGui/imgui.h>

PointLights::PointLights()
{
	Create();
}

PointLights::~PointLights()
{
	m_cbPointLights.reset();
}

void PointLights::Create()
{
	m_cbPointLights = std::make_unique<gfx::ConstantBuffer<cbPointLights>>(&m_cbPointLightsData);
	SetupLights();
}

void PointLights::SetupLights()
{
	m_LightPositions.resize(4);
	m_LightPositionsFloat.resize(4);

	m_LightColors.resize(4);
	m_LightColorsFloat.resize(4);

	m_LightPositions = {
		   DirectX::XMFLOAT4(-9.0f, +1.0f, 0.0f, 5.0f),
		   DirectX::XMFLOAT4(+0.0f, +1.0f, 0.0f, 5.0f),
		   DirectX::XMFLOAT4(+5.0f, +1.0f, 0.0f, 5.0f),
		   DirectX::XMFLOAT4(+9.0f, +1.0f, 0.0f, 5.0f)
	};

	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	for (size_t i = 0; i < m_LightColors.size(); i++)
	{
		m_LightColors.at(i) = {1.0f, 1.0f, 1.0f, 1.0f};
		m_LightColorsFloat.at(i) = {1.0f, 1.0f, 1.0f, 1.0f};
	}

	//m_PointLights.resize(4);

}

void PointLights::UpdateLights()
{
	for (size_t i = 0; i < m_Count; i++)
	{
		m_cbPointLightsData.LightPosition[i] = DirectX::XMFLOAT4(
			m_LightPositionsFloat.at(i).at(0), 
			m_LightPositionsFloat.at(i).at(1), 
			m_LightPositionsFloat.at(i).at(2), 
			m_LightPositionsFloat.at(i).at(3));
	
		m_cbPointLightsData.LightColor[i] = DirectX::XMFLOAT4(
			m_LightColorsFloat.at(i).at(0), 
			m_LightColorsFloat.at(i).at(1), 
			m_LightColorsFloat.at(i).at(2), 
			m_LightColorsFloat.at(i).at(3));
	}
	m_cbPointLights->Update(m_cbPointLightsData);
}

void PointLights::ResetLights()
{
	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	for (auto& color : m_LightColorsFloat)
		color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void PointLights::DrawGUI()
{
	ImGui::Begin("Point Lights");
	ImGui::Text("Positions");
	for (size_t i = 0; i < m_Count; i++)
	{
		auto label{ "Light #" + std::to_string(i) };
		ImGui::DragFloat4(label.c_str(), m_LightPositionsFloat.at(i).data());
	}

	for (size_t i = 0; i < m_Count; i++)
	{
		auto label{ "Color #" + std::to_string(i) };
		ImGui::ColorEdit4(label.c_str(), m_LightColorsFloat.at(i).data());
	}

	if (ImGui::Button("Reset Lights"))
	{
		ResetLights();
	}

	ImGui::End();
}
