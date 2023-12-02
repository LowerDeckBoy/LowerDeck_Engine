#include "TextureManager.hpp"
#include "../D3D/D3D12Context.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "../Utility/Utility.hpp"
#include "../Utility/FileSystem.hpp"
#include "TextureUtility.hpp"
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../D3D/D3D12Device.hpp"
#include "../D3D/D3D12Command.hpp"
#include "MipMapGenerator.hpp"

Texture TextureManager::Create(std::string_view Filepath)
{
	const auto extension{ utility::FileExtToEnum(Filepath) };

	Texture output;

	switch (extension)
	{
	case utility::FileExtension::eJPG:
	case utility::FileExtension::eJPEG:
	case utility::FileExtension::ePNG:
	{
		//CreateFromWIC(Filepath, &output);
		CreateWIC(Filepath, output);

		break;
	}
	case utility::FileExtension::eDDS:
	{
		// TODO: DDS
		break;
	}
	case utility::FileExtension::eHDR:
	{
		// TODO: HDR
		break;
	}

	default: // Invalid
		break;
	}

	return output;
}

void TextureManager::CreateFromWIC(const std::string_view& Filepath, Texture* pTarget)
{
	auto path{ utility::ToWideString(Filepath) };

	DirectX::ResourceUploadBatch upload(D3D::g_Device.Get());
	upload.Begin();

	ThrowIfFailed(DirectX::CreateWICTextureFromFileEx(
		D3D::g_Device.Get(),
		upload, path.c_str(), 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		DirectX::DX12::WIC_LOADER_IGNORE_SRGB | DirectX::DX12::WIC_LOADER_MIP_RESERVE, // 
		pTarget->m_Resource.GetAddressOf()));

	const auto desc{ pTarget->m_Resource->GetDesc() };
	uint16_t mipLevel = desc.MipLevels;

	// Generate Mipmaps ONLY for those textures which have SUITABLE format.
	// Else hardcode mip levels to 1, so SRVs are generated correctly.
	if (upload.IsSupportedForGenerateMips(desc.Format))
		upload.GenerateMips(pTarget->m_Resource.Get());
	else
		mipLevel = 1;

	auto finish{ upload.End(D3D::g_CommandQueue.Get()) };
	finish.wait();
	CreateSRV(pTarget->m_Resource.GetAddressOf(), pTarget->m_SRV, mipLevel, desc.Format);
	
	pTarget->Width = static_cast<uint32_t>(desc.Width);
	pTarget->Height = desc.Height;
	pTarget->MipLevels = mipLevel;
}

void TextureManager::CreateWIC(const std::string_view& Filepath, Texture& Target, bool bMipMaps)
{
	auto path{ utility::ToWideString(Filepath) };

	DirectX::ResourceUploadBatch upload(D3D::g_Device.Get());
	upload.Begin();

	D3D12_RESOURCE_FLAGS resourceFlags = (bMipMaps) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
	DirectX::DX12::WIC_LOADER_FLAGS loadingFlags{ DirectX::DX12::WIC_LOADER_IGNORE_SRGB };
	if (bMipMaps) loadingFlags |= DirectX::DX12::WIC_LOADER_MIP_RESERVE;

	ThrowIfFailed(DirectX::CreateWICTextureFromFileEx(
		D3D::g_Device.Get(),
		upload, path.c_str(), 0,
		resourceFlags,
		loadingFlags,
		Target.m_Resource.GetAddressOf()));

	const auto desc{ Target.m_Resource->GetDesc() };
	//uint16_t mipLevel = desc.MipLevels;

	Target.MipLevels = CountMips(static_cast<uint32_t>(desc.Width), desc.Height);
	Target.Desc   = desc;
	Target.Width  = static_cast<uint32_t>(desc.Width);
	Target.Height = desc.Height;

	auto finish{ upload.End(D3D::g_CommandQueue.Get()) };
	finish.wait();

	if (bMipMaps)
		MipMapGenerator::Generate2D(Target);

}

uint16_t TextureManager::CountMips(uint32_t Width, uint32_t Height)
{
	uint16_t count = 1;
	while (Width > 1 || Height > 1)
	{
		Width = Width >> 1;
		Height = Height >> 1;
		count++;
	}

	return count;
}

void TextureManager::CreateFromHDR(const std::string_view& , Texture* )
{
}

void TextureManager::CreateSRV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Mips, DXGI_FORMAT Format, D3D12_SRV_DIMENSION ViewDimension)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	if (ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = Mips;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}
	else if (ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = Mips;
		srvDesc.TextureCube.MostDetailedMip = 0;
	}

	D3D::D3D12Context::GetMainHeap()->Allocate(Descriptor);
	D3D::g_Device.Get()->CreateShaderResourceView((*ppResource), &srvDesc, Descriptor.GetCPU());
}

void TextureManager::CreateUAV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Depth, DXGI_FORMAT Format)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = Format;
	if (Depth == 1)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
	}
	else if (Depth == 6)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = Depth;
	}

	D3D::g_Device.Get()->CreateUnorderedAccessView((*ppResource), nullptr, &uavDesc, Descriptor.GetCPU());
}
