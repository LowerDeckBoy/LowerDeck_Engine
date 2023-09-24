#include "D3D12Viewport.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Command.hpp"
#include "D3D12Device.hpp"
#include "../Window/Window.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	ComPtr<IDXGISwapChain3> g_SwapChain{ nullptr };
	std::unique_ptr<D3D12DescriptorHeap> g_RenderTargetHeap{ nullptr };
	std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> g_RenderTargets;
	DXGI_FORMAT g_RenderTargetFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };

	ComPtr<ID3D12Fence1> g_Fence{ nullptr };
	HANDLE g_FenceEvent{ nullptr };
	std::array<uint64_t, FRAME_COUNT> g_FenceValues; 

	std::array<float, 4> g_ClearColor = { 0.5f, 0.5f, 1.0f, 1.0f };

	ID3D12Resource* GetRenderTarget()
	{
		return g_RenderTargets.at(FRAME_INDEX).Get();
	}

	bool InitializeSwapChain(D3D12Viewport Viewport)
	{
		g_RenderTargetHeap = std::make_unique<D3D12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_COUNT, L"Render Target Descriptor Heap");

		CreateFence();
		CreateSwapChain(Viewport);
		CreateBackbuffers();

		return true;
	}

	void CreateSwapChain(D3D12Viewport Viewport)
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.AlphaMode		= DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.BufferCount	= FRAME_COUNT;
		desc.Width			= static_cast<uint32_t>(Viewport.Viewport().Width);
		desc.Height			= static_cast<uint32_t>(Viewport.Viewport().Height);
		desc.SampleDesc		= { 1, 0 };

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc{};
		fullScreenDesc.Windowed = TRUE;
		fullScreenDesc.RefreshRate = { 0, 0 };
		fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		IDXGISwapChain1* swapChain{ nullptr };
		ThrowIfFailed(g_Factory.Get()->CreateSwapChainForHwnd(g_CommandQueue.Get(), Window::GetHwnd(), &desc, &fullScreenDesc, nullptr, &swapChain));
		ThrowIfFailed(swapChain->QueryInterface(g_SwapChain.ReleaseAndGetAddressOf()));
		SAFE_DELETE(swapChain);

		ThrowIfFailed(g_Factory.Get()->MakeWindowAssociation(Window::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

		FRAME_INDEX = g_SwapChain.Get()->GetCurrentBackBufferIndex();
	}

	void CreateBackbuffers()
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_RenderTargetHeap->Heap()->GetCPUDescriptorHandleForHeapStart());
		for (uint32_t i = 0; i < FRAME_COUNT; i++)
		{
			ThrowIfFailed(g_SwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(g_RenderTargets.at(i).ReleaseAndGetAddressOf())));
			g_Device.Get()->CreateRenderTargetView(g_RenderTargets.at(i).Get(), nullptr, rtvHandle);
			std::wstring debugName{ L"Backbuffer #" + std::to_wstring(i) };
			g_RenderTargets.at(i).Get()->SetName(debugName.c_str());
			rtvHandle.Offset(1, g_RenderTargetHeap->GetDescriptorSize());
		}
	}

	void ReleaseSwapChain()
	{
		for (auto& renderTarget : g_RenderTargets)
			SAFE_RELEASE(renderTarget);

		// Reset manually to avoid false-positive D3D12 WARNING
		g_RenderTargetHeap.reset();
		g_RenderTargetHeap = nullptr;

		SAFE_RELEASE(g_Fence);
		SAFE_RELEASE(g_SwapChain);
	}

	void CreateFence()
	{
		ThrowIfFailed(g_Device.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(g_Fence.ReleaseAndGetAddressOf())),
			"Failed to create Fence object!");
		g_FenceValues.at(0)++;

		g_FenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
		assert(g_FenceEvent != nullptr);
	}
}
