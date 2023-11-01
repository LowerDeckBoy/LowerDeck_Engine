#include "MipMapGenerator.hpp"
#include "../D3D/D3D12Device.hpp"
#include "../D3D/D3D12Descriptor.hpp"
#include "../D3D/D3D12Context.hpp"
#include "TextureUtility.hpp"
#include "../Utility/Math.hpp"
#include "../Utility/Utility.hpp"
#include "Shader5.hpp"
#include <algorithm>


#include <DirectXMath.h>
using namespace DirectX;

ID3D12RootSignature* MipMapGenerator::m_RootSignature = nullptr;
ID3D12PipelineState* MipMapGenerator::m_ComputePipeline = nullptr;
ID3D12Resource*		 MipMapGenerator::m_Textures[4] = {};

D3D::D3D12Descriptor MipMapGenerator::srvDescriptors = {};
D3D::D3D12Descriptor MipMapGenerator::uavDescriptors = {};

void MipMapGenerator::CreateComputeState(std::weak_ptr<gfx::ShaderManager> wpShaderManager)
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[2] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, 0);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4096, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {}; // 3
	rootParameters[0].InitAsConstants(8, 0);
	//rootParameters[1].InitAsDescriptorTable(1, &ranges[0]);
	//rootParameters[2].InitAsDescriptorTable(1, &ranges[1]);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_ANISOTROPIC;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	staticSamplers[0].MinLOD = 0.0f;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].MipLODBias = 0;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].RegisterSpace = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {};
	versionedRootSignatureDesc.Init_1_1(_countof(rootParameters), &rootParameters[0], _countof(staticSamplers), &staticSamplers[0], rootSignatureFlags);

	ComPtr<ID3DBlob> serializedRootSig;
	ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &serializedRootSig, &errorBlob);
	if (!SUCCEEDED(hr) || errorBlob)
	{
		//LOG_ERR(static_cast<const char*>(errorBlob->GetBufferPointer()));
		errorBlob->Release();
	}

	ThrowIfFailed(D3D::g_Device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
													 serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
	//s_Data.MipMapGenRootSig->SetName(StringHelper::StringToWString("Mip map root signature").c_str());

	gfx::Shader computeShader = wpShaderManager.lock()->CompileDXIL("Shaders/MipMaps_CS.hlsl", ShaderType::eCompute);
	//s_Data.MipMapGenShader = std::make_unique<Shader>(L"Resources/Shaders/MipMapGen_CS.hlsl", "main", "cs_6_0");

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	//psoDesc.pRootSignature = s_Data.MipMapGenRootSig.Get();
	//psoDesc.CS = s_Data.MipMapGenShader->GetShaderByteCode();
	psoDesc.pRootSignature = m_RootSignature;
	psoDesc.CS = computeShader.Bytecode();
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	//DX_CALL(s_Data.D3D12Device2->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&s_Data.MipMapGenPSO)));
	ThrowIfFailed(D3D::g_Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_ComputePipeline)));

}

void MipMapGenerator::Generate2D(Texture& Texture)
{
	auto& srcResource = Texture.m_Resource;
	if (!srcResource)
		return;

	auto srcResourceDesc = Texture.m_Resource->GetDesc();

	if (Texture.MipLevels == 1 || Texture.Width == 1)
		return;

	if (srcResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
		srcResourceDesc.DepthOrArraySize != 1 ||
		srcResourceDesc.SampleDesc.Count > 1)
	{
		ASSERT(false, "Generating mips for texture dimensions that are not TEXTURE2D, texture arrays, or multisampled textures is not supported");
		return;
	}

	ID3D12Device2* d3d12Device = D3D::g_Device.Get();
	ComPtr<ID3D12Resource> uavResource = Texture.m_Resource;
	ComPtr<ID3D12Resource> aliasResource;

	struct MipGenCB
	{
		uint32_t SrcMipIndex;
		uint32_t DestMipIndex;
		uint32_t SrcMipLevel;
		uint32_t NumMips;
		uint32_t SrcDimension;
		uint32_t IsSRGB;
		XMFLOAT2 TexelSize;
	} mipGenCB{};


	D3D::D3D12Context::GetMainHeap()->Allocate(Texture.m_SRV, 1);
	D3D::D3D12Context::GetMainHeap()->Allocate(Texture.m_UAV, Texture.MipLevels);

	D3D::g_CommandList->SetPipelineState(m_ComputePipeline);
	D3D::g_CommandList->SetComputeRootSignature(m_RootSignature);

	mipGenCB.IsSRGB = srcResourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? 1 : 0;

	//for (size_t srcMip = 0; srcMip < (srcResourceDesc.MipLevels - 1);)
	for (size_t srcMip = 0; srcMip < (size_t)(Texture.MipLevels - 1); srcMip++) // srcMip++
	{
		uint64_t srcWidth   = srcResourceDesc.Width >> srcMip;
		uint32_t srcHeight  = srcResourceDesc.Height >> srcMip;
		uint32_t destWidth  = static_cast<uint32_t>(srcWidth >> 1);
		uint32_t destHeight = srcHeight >> 1;

		mipGenCB.SrcDimension = uint32_t(((uint32_t)srcHeight & 1) << 1 | (srcWidth & 1));
		DWORD mipCount = 0;

		_BitScanForward(&mipCount, (destWidth == 1 ? destHeight : destWidth) |
						(destHeight == 1 ? destWidth : destHeight));

		//mipCount = std::min<DWORD>(4, mipCount - 1);
		mipCount = std::min<DWORD>(1, mipCount - 1);
		mipCount = (srcMip + mipCount) >= Texture.MipLevels ? Texture.MipLevels - srcMip - 1 : mipCount;
	
		destWidth  = std::max(1u, destWidth);
		destHeight = std::max(1u, destHeight);

		mipGenCB.SrcMipLevel = srcMip;
		mipGenCB.NumMips = mipCount;
		mipGenCB.TexelSize.x = 1.0f / (float)destWidth;
		mipGenCB.TexelSize.y = 1.0f / (float)destHeight;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = srcResourceDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = Texture.MipLevels;// - 1

		d3d12Device->CreateShaderResourceView(uavResource.Get(), &srvDesc, Texture.m_SRV.GetCPU());

		for (size_t mip = 0; mip < mipCount; ++mip)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = srcResourceDesc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

			d3d12Device->CreateUnorderedAccessView(uavResource.Get(), nullptr, &uavDesc, { Texture.m_UAV.GetCPU().ptr + ((srcMip + mip) * d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) });
		}

		D3D::g_CommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());

		mipGenCB.SrcMipIndex  = D3D::D3D12Context::GetMainHeap()->GetIndexFromOffset(Texture.m_SRV, 0);
		mipGenCB.DestMipIndex = D3D::D3D12Context::GetMainHeap()->GetIndexFromOffset(Texture.m_UAV, srcMip);

		D3D::g_CommandList->SetComputeRoot32BitConstants(0, 8, &mipGenCB, 0);

		//uint32_t dispatchers{ (destWidth + 8u - 1) / 8u };
		//D3D::g_CommandList->Dispatch(dispatchers, dispatchers, 1);
		D3D::g_CommandList->Dispatch(((destWidth + 8u - 1) / 8u), (destHeight + 8u - 1) / 8u, 1);

		CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(uavResource.Get());
		D3D::g_CommandList->ResourceBarrier(1, &uavBarrier);
	}

	D3D::ExecuteCommandLists(true);
}

/*
void MipMapGenerator::Test(Texture& Texture)
{
	auto srcResource = Texture.m_Resource;
	if (!srcResource)
		return;

	auto srcResourceDesc = srcResource->GetDesc();
	//if (srcResourceDesc.MipLevels == 1)
	if (Texture.MipLevels == 1 || Texture.Width == 1)
		return;

	if (srcResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
		srcResourceDesc.DepthOrArraySize != 1 ||
		srcResourceDesc.SampleDesc.Count > 1)
	{
		ASSERT(false, "Generating mips for texture dimensions that are not TEXTURE2D, texture arrays, or multisampled textures is not supported");
		return;
	}

	ID3D12Device2* d3d12Device = D3D::g_Device.Get();
	ComPtr<ID3D12Resource> uavResource = srcResource;
	ComPtr<ID3D12Resource> aliasResource;

	/*
	if ((srcResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		D3D12_RESOURCE_DESC aliasDesc = srcResourceDesc;
		aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_RESOURCE_DESC uavDesc = aliasDesc;
		D3D12_RESOURCE_DESC aliasUAVDescs[] = { aliasDesc, uavDesc };

		D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = d3d12Device->GetResourceAllocationInfo(0, _countof(aliasUAVDescs), aliasUAVDescs);

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

		ComPtr<ID3D12Heap> heap;
		ThrowIfFailed(d3d12Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));
		//TrackObject(heap);

		ThrowIfFailed(d3d12Device->CreatePlacedResource(
			heap.Get(),
			0,
			&aliasDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&aliasResource)
		));
		//TrackObject(aliasResource);

		ThrowIfFailed(d3d12Device->CreatePlacedResource(
			heap.Get(),
			0,
			&uavDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&uavResource)
		));
		//TrackObject(uavResource);

		CD3DX12_RESOURCE_BARRIER aliasBarrier = CD3DX12_RESOURCE_BARRIER::Aliasing(nullptr, aliasResource.Get());
		D3D::g_CommandList->ResourceBarrier(1, &aliasBarrier);

		D3D::g_CommandList->CopyResource(aliasResource.Get(), srcResource.Get());

		CD3DX12_RESOURCE_BARRIER uavAliasBarrier = CD3DX12_RESOURCE_BARRIER::Aliasing(aliasResource.Get(), uavResource.Get());
		D3D::g_CommandList->ResourceBarrier(1, &uavAliasBarrier);
	}
	

struct MipGenCB
{
	uint32_t SrcMipIndex;
	uint32_t DestMipIndex;
	uint32_t SrcMipLevel;
	uint32_t NumMips;
	uint32_t SrcDimension;
	uint32_t IsSRGB;
	XMFLOAT2 TexelSize;
} mipGenCB;


D3D::D3D12Context::GetMainHeap()->Allocate(Texture.m_SRV, 1);
D3D::D3D12Context::GetMainHeap()->Allocate(Texture.m_UAV, Texture.MipLevels);
//D3D::D3D12Context::GetMainHeap()->Allocate(Texture.m_UAV, srcResourceDesc.MipLevels);

//D3D::D3D12Context::GetMainHeap()->Allocate(srvDescriptors, 1);
//D3D::D3D12Context::GetMainHeap()->Allocate(uavDescriptors, srcResourceDesc.MipLevels);

D3D::g_CommandList->SetPipelineState(m_ComputePipeline);
D3D::g_CommandList->SetComputeRootSignature(m_RootSignature);

mipGenCB.IsSRGB = srcResourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? 1 : 0;

//for (size_t srcMip = 0; srcMip < (srcResourceDesc.MipLevels - 1);)
for (size_t srcMip = 0; srcMip < (Texture.MipLevels - 1);) // ++srcMip
{
	uint64_t srcWidth = srcResourceDesc.Width >> srcMip;
	uint32_t srcHeight = srcResourceDesc.Height >> srcMip;
	uint32_t destWidth = static_cast<uint32_t>(srcWidth >> 1);
	uint32_t destHeight = srcHeight >> 1;

	mipGenCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);
	DWORD mipCount = 0;

	_BitScanForward(&mipCount, (destWidth == 1 ? destHeight : destWidth) |
					(destHeight == 1 ? destWidth : destHeight));

	mipCount = std::min<DWORD>(4, mipCount);
	mipCount = (srcMip + mipCount) >= Texture.MipLevels ? Texture.MipLevels - srcMip - 1 : mipCount;

	destWidth = std::max(1u, destWidth);
	destHeight = std::max(1u, destHeight);

	mipGenCB.SrcMipLevel = srcMip;
	mipGenCB.NumMips = mipCount;
	mipGenCB.TexelSize.x = 1.0f / (float)destWidth;
	mipGenCB.TexelSize.y = 1.0f / (float)destHeight;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = srcResourceDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = Texture.MipLevels;

	d3d12Device->CreateShaderResourceView(uavResource.Get(), &srvDesc, Texture.m_SRV.GetCPU());
	//d3d12Device->CreateShaderResourceView(uavResource.Get(), &srvDesc, srvDescriptors.GetCPU());

	for (size_t mip = 0; mip < mipCount; ++mip)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		//uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.Format = srcResourceDesc.Format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

		d3d12Device->CreateUnorderedAccessView(uavResource.Get(), nullptr, &uavDesc, { Texture.m_UAV.GetCPU().ptr + ((srcMip + mip) * d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) });
	}

	D3D::g_CommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());

	mipGenCB.SrcMipIndex = D3D::D3D12Context::GetMainHeap()->GetIndexFromOffset(Texture.m_SRV, 0);
	mipGenCB.DestMipIndex = D3D::D3D12Context::GetMainHeap()->GetIndexFromOffset(Texture.m_UAV, srcMip);

	D3D::g_CommandList->SetComputeRoot32BitConstants(0, 8, &mipGenCB, 0);

	//uint32_t dispatchers{ (destWidth + 8u - 1) / 8u };
	//D3D::g_CommandList->Dispatch(dispatchers, dispatchers, 1);
	D3D::g_CommandList->Dispatch(((destWidth + 8u - 1) / 8u), (destHeight + 8u - 1) / 8u, 1);

	CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(uavResource.Get());
	D3D::g_CommandList->ResourceBarrier(1, &uavBarrier);

	srcMip += mipCount;
}


D3D::ExecuteCommandLists(true);
debug::Print("Mip generated\n");
}
*/

/*
void MipMapGenerator::Generate(Texture& Texture, uint16_t MipLevels, uint32_t Depth)
{
	//const auto desc = Texture.m_Resource->GetDesc();

	for (uint32_t slice = 0; slice < MipLevels; slice++)
	{
		D3D::D3D12Descriptor srvDescriptor{ };
		D3D::D3D12Context::GetMainHeap()->Allocate(srvDescriptor, 1);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = MipLevels;
		srvDesc.Texture2DArray.ArraySize = 1;
		srvDesc.Texture2DArray.FirstArraySlice = slice;
		D3D::g_Device->CreateShaderResourceView(Texture.m_Resource.Get(), &srvDesc, srvDescriptor.GetCPU());



	}

}
*/
