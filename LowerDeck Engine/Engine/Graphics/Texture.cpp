#include "../D3D/D3D12Descriptor.hpp"
#include "Texture.hpp"
#include "TextureUtility.hpp"
#include "../Utility/FileSystem.hpp"
#include "../Utility/Utility.hpp"


Texture::Texture(const std::string_view& Filepath)
{
	Create(Filepath);
}

Texture::~Texture()
{
	Release();
}

void Texture::Create(const std::string_view& Filepath)
{
	const auto extension{ utility::FileExtToEnum(Filepath) };

	switch (extension)
	{
	case utility::FileExtension::eJPG:
	case utility::FileExtension::eJPEG:
	case utility::FileExtension::ePNG:
	{
		// WIC
		//TextureUtility::CreateFromWIC(Filepath, m_Resource.GetAddressOf(), m_SRV);
		break;
	}
	case utility::FileExtension::eDDS:
	{
		// TODO: DDS
		break;
	}
	case utility::FileExtension::eHDR:
	{
		//TextureUtility::CreateFromHDR(Filepath, m_Resource.GetAddressOf());
		break;
	}

	default: // Invalid
		break;
	}

}

uint32_t Texture::CountMips()
{
	if (Width == 0 || Height == 0)
		return 0;

	uint32_t count = 1;
	while (Width > 1 || Height > 1)
	{
		Width >>= 1;
		Height >>= 1;
		count++;
	}
	return count;
}

void Texture::Release()
{
	SAFE_RELEASE(m_Resource);
}
