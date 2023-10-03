#include "TextureUtility.hpp"
#include <DirectXTex.h>
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../D3D/D3D12Context.hpp"
//#include "../D3D/D3D12Device.hpp"
//#include "../D3D/D3D12Command.hpp"
//#include "../D3D/D3D12Utility.hpp"


void TextureUtility::CreateFromWIC(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor)
{
	std::wstring wpath = std::wstring(Filepath.begin(), Filepath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	DirectX::LoadFromWICFile(path,
		DirectX::WIC_FLAGS_FORCE_SRGB,
		nullptr,
		*scratchImage);

	// https://github.com/microsoft/DirectXTK12/wiki/ResourceUploadBatch
	auto* upload = new DirectX::ResourceUploadBatch(D3D::g_Device.Get());
	upload->Begin();

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource{};

	ThrowIfFailed(DirectX::LoadWICTextureFromFileEx(D3D::g_Device.Get(), path, 0, D3D12_RESOURCE_FLAG_NONE, DirectX::DX12::WIC_LOADER_MIP_AUTOGEN, ppResource, decodedData, subresource));

	const auto desc{ (*ppResource)->GetDesc()};

	const uint16_t mips = desc.MipLevels;
	const auto uploadDesc = CD3DX12_RESOURCE_DESC(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		desc.Width, desc.Height, 1, mips,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1, 0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE);

	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&D3D::Utility::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(ppResource)));

	upload->Upload((*ppResource), 0, &subresource, 1);
	upload->Transition((*ppResource), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	upload->GenerateMips((*ppResource));

	auto finish{ upload->End(D3D::g_CommandQueue.Get()) };
	finish.wait();
	CreateSRV(ppResource, Descriptor, desc.MipLevels);
	
	delete upload;
	delete scratchImage;
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
