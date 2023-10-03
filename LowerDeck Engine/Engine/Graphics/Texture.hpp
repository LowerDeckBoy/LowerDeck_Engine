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
	Texture(const Texture&) = delete;
	Texture(const Texture&&) = delete;
	Texture operator=(const Texture&) = delete;
	~Texture();

	void Create(const std::string_view& Filepath);

	void Release();

	D3D::D3D12Descriptor SRV() { return m_SRV; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadResource;

	D3D::D3D12Descriptor m_SRV;
	//D3D::D3D12Descriptor m_UAV;

	uint32_t m_Width{};
	uint32_t m_Height{};
	uint16_t m_MipLevels{ 1 };

	DXGI_FORMAT m_Format{ DXGI_FORMAT_R8G8B8A8_UNORM };

};
