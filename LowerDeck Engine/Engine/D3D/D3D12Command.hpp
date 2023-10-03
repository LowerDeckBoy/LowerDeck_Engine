#pragma once
#include <d3d12.h>
#include "D3D12Utility.hpp"
#include "D3D12Types.hpp"
#include "../Utility/Utility.hpp"

using Microsoft::WRL::ComPtr;

namespace D3D
{
	class D3D12RootSignature;
	class D3D12PipelineState;

	extern ComPtr<ID3D12GraphicsCommandList4>	g_CommandList;
	extern ComPtr<ID3D12CommandQueue>			g_CommandQueue;
	extern ComPtr<ID3D12CommandQueue>			g_CommandComputeQueue;

	extern std::array<ComPtr<ID3D12CommandAllocator>, FRAME_COUNT> g_CommandAllocators;

	/// <summary>
	/// Create Command Allocators (one per frame), Graphics Command List and Graphics Command Queue.
	/// </summary>
	void InitializeCommands();
	/// <summary>
	/// Safe release of <c>ComPtrs</c>.
	/// </summary>
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

	/// <summary>
	/// Wrapper for setting Root Signatures.
	/// </summary>
	/// <param name="RootSignature"> Target object. </param>
	/// <param name="Usage"> Is meant for Graphics or Compute pipeline usage. </param>
	void SetRootSignature(D3D12RootSignature& RootSignature, PipelineType Usage = PipelineType::eGraphics);

	/// <summary>
	/// Wrapper for setting Pipeline State Objects.
	/// </summary>
	/// <param name="Usage"> Target object. </param>
	void SetPSO(D3D12PipelineState& PSO);

	/// <summary>
	/// Single Resource barrier transition.
	/// </summary>
	/// <param name="pResource"> Target resource to make transition. </param>
	/// <param name="Before"> Current state. </param>
	/// <param name="After"> Desired state. </param>
	void TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);

	/// <returns> Command Allocator for CURRENT frame. </returns>
	inline ID3D12CommandAllocator* GetCommandAllocator() { return g_CommandAllocators.at(FRAME_INDEX).Get(); }
}
