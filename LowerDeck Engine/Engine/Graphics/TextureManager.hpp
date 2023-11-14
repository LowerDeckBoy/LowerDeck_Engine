#pragma once
#include <unordered_map>
#include "Texture.hpp"
#include "../Utility/Singleton.hpp"


class TextureManager : public Singleton<TextureManager>
{
	friend class Singleton<TextureManager>;
public:

	//void Create(Texture& Tex);
	Texture Create(std::string_view Filepath);

private:
	// Create2D
	void CreateFromWIC(const std::string_view& Filepath, Texture* pTarget);
	void CreateWIC(const std::string_view& Filepath, Texture& Target, bool bMipMaps = true);

	uint16_t CountMips(uint32_t Width, uint32_t Height);

	void CreateFromHDR(const std::string_view& Filepath, Texture* pTarget);


	void CreateSRV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Mips, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_SRV_DIMENSION ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D);

	void CreateUAV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Depth, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

private:
	// TODO: 
	// Gather all textures here and only distribute Indices to models
	std::unordered_map<uint32_t, Texture*> m_Textures;
	//std::vector<Texture*> m_Textures; 

};
