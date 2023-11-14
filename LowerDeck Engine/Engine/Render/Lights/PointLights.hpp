#pragma once
#include "../../Graphics/Buffer/ConstantBuffer.hpp"

struct cbPointLights
{
	DirectX::XMFLOAT4 LightPosition[4];
	DirectX::XMFLOAT4 LightColor[4];
};

class PointLights
{
public:
	PointLights();
	~PointLights();

	void Create();

	void SetupLights();
	void UpdateLights();
	void ResetLights();

	void DrawGUI();

	std::unique_ptr<gfx::ConstantBuffer<cbPointLights>> m_cbPointLights;
	cbPointLights m_cbPointLightsData{};

private:
	std::vector<DirectX::XMFLOAT4>		m_LightPositions{};
	std::vector<std::array<float, 4>>   m_LightPositionsFloat{};
	std::vector<DirectX::XMFLOAT4>		m_LightColors{};
	std::vector<std::array<float, 4>>	m_LightColorsFloat{};

	const uint32_t m_Count{ 4 };

};
