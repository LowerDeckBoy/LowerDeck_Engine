#include "TextureUtility.hpp"
#include <DirectXTex.h>
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../D3D/D3D12Context.hpp"
#include "../Utility/FileSystem.hpp"
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

void TextureUtility::CreateFromHDR(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, bool bGenerateMipChain)
{
	if (utility::GetFileExtension(Filepath) != ".hdr")
	{
		// message here
	}

	std::wstring path{ utility::ToWideString(Filepath) };
	DirectX::ScratchImage scratchImage{};
	DirectX::LoadFromHDRFile(path.c_str(), nullptr, scratchImage);

	// TEST
	//if (bGenerateMipChain)
	//{
	//	scratchImage = GenerateMipChain(scratchImage, 6, false);
	//}
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

	D3D12_RESOURCE_DESC desc{};
	desc.Format		= metadata.format;
	desc.Width		= static_cast<uint32_t>(metadata.width);
	desc.Height		= static_cast<uint32_t>(metadata.height);
	desc.MipLevels	= scratchImage.GetMetadata().mipLevels;
	//desc.MipLevels	= 1;
	desc.DepthOrArraySize = metadata.IsCubemap() ? static_cast<uint16_t>(metadata.arraySize) : static_cast<uint16_t>(metadata.depth);
	desc.SampleDesc = { 1, 0 };
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&D3D::Utility::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(ppResource)));

	const uint64_t bufferSize{ ::GetRequiredIntermediateSize((*ppResource), 0, 1) };

	ID3D12Resource* uploadHeap{ nullptr };
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&D3D::Utility::HeapUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap)));

	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData		= scratchImage.GetImages()->pixels;
	subresource.RowPitch	= static_cast<int64_t>(scratchImage.GetImages()->rowPitch);
	subresource.SlicePitch	= static_cast<int64_t>(scratchImage.GetImages()->slicePitch);

	::UpdateSubresources(D3D::g_CommandList.Get(), (*ppResource), uploadHeap, 0, 0, 1, &subresource);

	D3D::ExecuteCommandLists(true);

	D3D::TransitResource((*ppResource), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	// Delete AFTER executing Command Lists, otherwise upload resource will remain live.
	SAFE_DELETE(uploadHeap);

}

ID3D12Resource* TextureUtility::CreateResource(TextureData Data, TextureDesc Desc)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags		= Desc.Flag;
	resourceDesc.Format		= Data.Format;
	resourceDesc.Width		= Data.Width;
	resourceDesc.Height		= Data.Height;
	resourceDesc.Dimension	= Data.Dimension;
	resourceDesc.MipLevels	= 1;
	resourceDesc.DepthOrArraySize = Data.Depth;
	resourceDesc.SampleDesc = { 1, 0 };

	ID3D12Resource* resource{ nullptr };
	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(&resource)
	));

	return resource;
}

void TextureUtility::CreateResource(ID3D12Resource** ppTarget, TextureData Data, TextureDesc Desc)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags		= Desc.Flag;
	resourceDesc.Format		= Data.Format;
	resourceDesc.Width		= Data.Width;
	resourceDesc.Height		= Data.Height;
	resourceDesc.Dimension	= Data.Dimension;
	resourceDesc.MipLevels	= Data.MipLevels;
	resourceDesc.DepthOrArraySize = Data.Depth;
	resourceDesc.SampleDesc = { 1, 0 };

	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(ppTarget)
	));
}

void TextureUtility::CreateFromWIC(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, bool bMips)
{
	auto path{ utility::ToWideString(Filepath) };

	DirectX::ResourceUploadBatch upload(D3D::g_Device.Get());
	upload.Begin();
	//DirectX::LoadWICTextureFromMemoryEx()
	//ThrowIfFailed(DirectX::LoadFro)

}

DirectX::ScratchImage TextureUtility::GenerateMipChain(DirectX::ScratchImage& BaseImage, uint16_t MipLevels, bool bIsWIC)
{
	DirectX::ScratchImage mipChain;
	DirectX::TexMetadata metadata{};

	auto flags{ (!bIsWIC) ? DirectX::TEX_FILTER_FORCE_NON_WIC : DirectX::TEX_FILTER_DEFAULT };
	ThrowIfFailed(DirectX::GenerateMipMaps(BaseImage.GetImages(), BaseImage.GetImageCount(), BaseImage.GetMetadata(), flags, (size_t)MipLevels, mipChain));
	
	return mipChain;
}

void TextureUtility::CreateSRV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Mips, DXGI_FORMAT Format, D3D12_SRV_DIMENSION ViewDimension)
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

void TextureUtility::CreateUAV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Depth, DXGI_FORMAT Format)
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
