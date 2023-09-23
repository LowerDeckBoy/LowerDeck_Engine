#pragma once
#include <d3d12.h>
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

using Microsoft::WRL::ComPtr;

namespace D3D
{
	extern ComPtr<ID3D12GraphicsCommandList4>	g_CommandList;
	extern ComPtr<ID3D12CommandQueue>			g_CommandQueue;
	extern ComPtr<ID3D12CommandQueue>			g_CommandComputeQueue;

	extern std::array<ComPtr<ID3D12CommandAllocator>, FRAME_COUNT> g_CommandAllocators;

	bool InitializeCommands();
	void ReleaseCommands();

	/// <summary> Execute content currently bound to ID3D12GraphicsCommandList4*. </summary>
	/// <param name="bResetAllocator">: If true Close command list. False by default.</param>
	void ExecuteCommandLists(bool bResetAllocator = false);

	/// <summary>
	/// Reset current ID3D12CommandAllocator and ID3D12GraphicsCommandList.<br/>
	/// Used before Command List starts preparing current frame - 
	/// begins BeginFrame method and thus starts recording Command List content.
	/// </summary>
	void ResetCommandLists();

	/// <returns> Command Allocator for CURRENT frame. </returns>
	inline ID3D12CommandAllocator* GetCommandAllocator() { return g_CommandAllocators.at(FRAME_INDEX).Get(); }
}
