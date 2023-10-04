#include "TextureUtility.hpp"
#include <DirectXTex.h>
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../D3D/D3D12Context.hpp"
#include "../Utility/Utility.hpp"


void TextureUtility::CreateFromWIC(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor)
{
	auto path{ utility::ToWideString(Filepath) };

	DirectX::ResourceUploadBatch upload(D3D::g_Device.Get());
	upload.Begin();

	ThrowIfFailed(DirectX::CreateWICTextureFromFileEx(
		D3D::g_Device.Get(), 
		upload, path.c_str(), 0,
		D3D12_RESOURCE_FLAG_NONE, 
		DirectX::DX12::WIC_LOADER_IGNORE_SRGB | DirectX::DX12::WIC_LOADER_MIP_RESERVE,
		ppResource));

	const auto desc{ (*ppResource)->GetDesc()};

	uint16_t mipLevel = desc.MipLevels;

	// Generate Mipmaps ONLY for those textures which have SUITABLE format.
	// Else hardcode mip levels to 1, so SRVs are generated correctly.
	if (upload.IsSupportedForGenerateMips(desc.Format))
		upload.GenerateMips((*ppResource));
	else
		mipLevel = 1;

	auto finish{ upload.End(D3D::g_CommandQueue.Get()) };
	finish.wait();
	CreateSRV(ppResource, Descriptor, mipLevel, desc.Format);

}

void TextureUtility::CreateFromDDS(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor)
{
}

void TextureUtility::CreateFromHDR(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor)
{
}

void TextureUtility::CreateSRV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Mips, DXGI_FORMAT Format)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = Mips;
	srvDesc.Texture2D.MostDetailedMip = 0;

	D3D::D3D12Context::GetMainHeap()->Allocate(Descriptor);
	D3D::g_Device.Get()->CreateShaderResourceView((*ppResource), &srvDesc, Descriptor.GetCPU());
}

void TextureUtility::CreateUAV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	
}
