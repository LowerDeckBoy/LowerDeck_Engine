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
	
	extern std::array<float, 4> g_ClearColor;

	/// <summary> Get Render Target for current frame. </summary>
	/// <returns>ID3D12Resource*</returns>
	ID3D12Resource* GetRenderTarget();

	bool InitializeSwapChain(D3D12Viewport Viewport);
	void CreateSwapChain(D3D12Viewport Viewport);
	void CreateBackbuffers();
	void CreateFence();
	// TODO:
	//void OnSwapChainResize(D3D12Viewport Viewport);

	void ReleaseSwapChain();
}
