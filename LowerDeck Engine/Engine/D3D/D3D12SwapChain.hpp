#pragma once
#include <dxgi1_6.h>
#include "D3D12Utility.hpp"

class D3D12Viewport;
class D3D12DescriptorHeap;

using Microsoft::WRL::ComPtr;

namespace D3D
{
	extern ComPtr<IDXGISwapChain3> g_SwapChain;

	extern std::unique_ptr<D3D12DescriptorHeap> g_RenderTargetHeap;
	extern std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> g_RenderTargets;

	extern DXGI_FORMAT g_RenderTargetFormat;

	extern ComPtr<ID3D12Fence1> g_Fence;
	extern HANDLE g_FenceEvent;
	extern std::array<uint64_t, FRAME_COUNT> g_FenceValues;
	/// Swap Chain backbuffer background color
	extern std::array<float, 4> g_ClearColor;

	/// <summary> Get Render Target for <b>current</b> frame. </summary>
	/// <returns><c>ID3D12Resource*</c></returns>
	ID3D12Resource* GetRenderTarget();

	bool InitializeSwapChain(D3D12Viewport Viewport);
	void CreateSwapChain(D3D12Viewport Viewport);
	void CreateBackbuffers();
	void CreateFence();

	void ReleaseSwapChain();
}
