#pragma once
#include <string>
#include "../D3D/D3D12Types.hpp"
#include "../D3D/D3D12Descriptor.hpp"
#include "../D3D/D3D12Utility.hpp"


class Texture
{
public:
	Texture() = default;
	Texture(const std::string_view& Filepath);
	//Texture(const Texture&) = delete;
	//Texture(const Texture&&) = delete;
	//Texture operator=(const Texture&) = delete;
	~Texture();

	void Create(const std::string_view& Filepath);
	void CreateResource();

	uint32_t CountMips();

	void Release();

	D3D::D3D12Descriptor SRV() { return m_SRV; }
	D3D::D3D12Descriptor UAV() { return m_UAV; }

	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;

	D3D::D3D12Descriptor m_SRV;
	D3D::D3D12Descriptor m_UAV;

	uint32_t Width { 0 };
	uint32_t Height{ 0 };
	uint16_t MipLevels{ 1 };

	DXGI_FORMAT Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D12_RESOURCE_DESC Desc{};

private:
	// For serialization purposes
	std::string m_Filepath;

};
