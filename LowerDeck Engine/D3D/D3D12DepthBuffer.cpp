#include "D3D12Context.hpp"
#include "D3D12DepthBuffer.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, DXGI_FORMAT Format, bool bSRV)
	{
		m_Format = Format;
		Create(pDepthHeap, pViewport, bSRV);
	}
	D3D12DepthBuffer::~D3D12DepthBuffer()
	{
		Release();
	}

	void D3D12DepthBuffer::Create(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, bool bSRV)
	{
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
		const auto heapDesc{ CD3DX12_RESOURCE_DESC::Tex2D(m_Format,
													static_cast<uint64_t>(pViewport->Viewport().Width),
													static_cast<uint32_t>(pViewport->Viewport().Height),
													1, 0, 1, 0,
													D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) };

		ThrowIfFailed(g_Device.Get()->CreateCommittedResource(&heapProperties,
															  D3D12_HEAP_FLAG_NONE,
															  &heapDesc,
															  D3D12_RESOURCE_STATE_DEPTH_WRITE,
															  &clearValue,
															  IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())),
																"Failed to create DepthStencil!");
		m_Resource.Get()->SetName(L"Depth Stencil");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
		dsView.Flags = D3D12_DSV_FLAG_NONE;
		dsView.Format = m_Format;
		dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsView.Texture2D.MipSlice = 0;

		pDepthHeap->Allocate(m_DSV);
		g_Device.Get()->CreateDepthStencilView(m_Resource.Get(), &dsView, m_DSV.GetCPU());

		if (bSRV)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (m_Format == DXGI_FORMAT_D32_FLOAT)
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
				srvDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

			D3D12Context::GetMainHeap()->Allocate(m_SRV);
			g_Device.Get()->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_SRV.GetCPU());
		}
	}
	void D3D12DepthBuffer::Clear()
	{
		const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DSV.GetCPU());
		g_CommandList.Get()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);
	}

	void D3D12DepthBuffer::OnResize(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport)
	{
		if (m_Resource.Get())
			m_Resource.Reset();

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
		const auto heapDesc{ CD3DX12_RESOURCE_DESC::Tex2D(m_Format,
													static_cast<uint64_t>(pViewport->Viewport().Width),
													static_cast<uint32_t>(pViewport->Viewport().Height),
													1, 0, 1, 0,
													D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) };

		ThrowIfFailed(g_Device.Get()->CreateCommittedResource(&heapProperties,
															  D3D12_HEAP_FLAG_NONE,
															  &heapDesc,
															  D3D12_RESOURCE_STATE_DEPTH_WRITE,
															  &clearValue,
															  IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())),
																"Failed to create DepthStencil!");
		m_Resource.Get()->SetName(L"Depth Stencil");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
		dsView.Flags = D3D12_DSV_FLAG_NONE;
		dsView.Format = m_Format;
		dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsView.Texture2D.MipSlice = 0;

		pDepthHeap->Allocate(m_DSV);
		g_Device.Get()->CreateDepthStencilView(m_Resource.Get(), &dsView, m_DSV.GetCPU());
	}

	void D3D12DepthBuffer::Release()
	{
		SAFE_RELEASE(m_Resource);
	}
}
