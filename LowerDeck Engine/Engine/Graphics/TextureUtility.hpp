#pragma once
#include <string_view>
#include <d3d12.h>
#include <wrl/client.h>

//struct ID3D12Resource;
namespace D3D
{
	class D3D12Descriptor;
}

/*
struct TextureData
{
	uint64_t Width;
	uint32_t Height;
	uint16_t Depth;
	DXGI_FORMAT	Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	uint16_t MipLevels{ 1 };
	D3D12_RESOURCE_DIMENSION Dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D };
};

struct TextureDesc
{
	D3D12_RESOURCE_FLAGS	Flag{ D3D12_RESOURCE_FLAG_NONE };
	D3D12_RESOURCE_STATES	State{ D3D12_RESOURCE_STATE_COMMON };
	//CD3DX12_HEAP_PROPERTIES HeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	D3D12_HEAP_FLAGS		HeapFlag{ D3D12_HEAP_FLAG_NONE };
};
*/
class TextureUtility
{
public:
	/// <summary>
	/// Create <c>Texture</c> from JPG, JPEG or PNG files.<br/>
	/// Generates MipMaps.
	/// </summary>
	/// <param name="Filepath"></param>
	/// <param name="ppResource"></param>
	/// <param name="Descriptor"></param>
	static void CreateFromWIC(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor);
	// Note: testing required
	static void CreateFromDDS(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor);
	// Note: testing required
	static void CreateFromHDR(const std::string_view& Filepath, ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor);

	//static ID3D12Resource* CreateResource();

	/// <summary>
	/// Create <c>ShaderResourceView</c> for given <c>Texture Resource</c> and it's <c>Descriptor</c>.
	/// </summary>
	/// <param name="ppResource"> Target Resource. Usually <c>ComPtr</c>. </param>
	/// <param name="Descriptor"> Target Descriptor. </param>
	/// <param name="Mips"> Level of MipMap chain.</param>
	/// <param name="Format"> Texture format. Note: not every format supports mipmap generation. </param>
	static void CreateSRV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, uint16_t Mips, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

	/// <summary>
	/// Create <c>UnorderedAccessView</c> for given <c>Texture Resource</c> and it's <c>Descriptor</c>.
	/// </summary>
	/// <param name="ppResource"> Target Resource. Usually <c>ComPtr</c>. </param>
	/// <param name="Descriptor"> Target Descriptor. </param>
	/// <param name="Format"> Texture format. </param>
	static void CreateUAV(ID3D12Resource** ppResource, D3D::D3D12Descriptor& Descriptor, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

};
