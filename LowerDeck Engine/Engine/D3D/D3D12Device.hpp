#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <d3d12sdklayers.h>
#include <D3D12MA/D3D12MemAlloc.h>
#include <wrl/client.h>
#include <cstdint>
#include "D3D12Descriptor.hpp"
#include "D3D12DescriptorHeap.hpp"

#if defined (_DEBUG)
#include <dxgidebug.h>
#endif

using Microsoft::WRL::ComPtr;

// Experimental approach
// Goal is to reduce amount of data passing
// whenever D3D context is required, via using static variables.
namespace D3D
{
	class D3D12Device
	{
		public:


	};

	/// <summary>
	/// Creates Adapter and Device objects.
	/// </summary>
	void InitializeDevice();
	/// <summary>
	/// Raytracing support is mendatory
	/// as engine is meant to be built around hybrid path
	/// </summary>
	bool CheckRaytracingSupport();
	/// <summary> GPU Virtual Memory usage. </summary>
	/// <returns> Current VRAM usage in MegaBytes. </returns>
	uint32_t QueryAdapterMemory();

	/// <summary>
	/// Make allocation from <c>D3D12MA::Allocator</c>.
	/// </summary>
	/// <param name="ppResource"> Target Resource. </param>
	/// <param name="pAllocation"> Target Allocation. </param>
	/// <param name="AllocationFlags"> D3D12MA Flags. </param>
	/// <param name="HeapDesc"> Heap Desc. </param>
	/// <param name="HeapType"> Desired Heap type. Defaults to <c>D3D12_HEAP_TYPE_UPLOAD</c>. </param>
	/// <param name="HeapFlags"> Defaults to <c>D3D12_HEAP_FLAG_NONE</c>. </param>
	void HeapAllocation(ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation, const CD3DX12_RESOURCE_DESC& HeapDesc, 
		D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY, 
		D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_UPLOAD, 
		D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAG_NONE);

	/// <summary> 
	/// Release all Device related ComPtrs.<br/>
	/// Should return Refcount: 1 for Device object.<br/>
	/// Otherwise some resources haven't been released, or were released incorrectly.
	/// </summary>
	void ReleaseDevice();

	extern ComPtr<IDXGIFactory7> g_Factory;
	extern ComPtr<IDXGIAdapter3> g_Adapter;
	extern ComPtr<ID3D12Device5> g_Device;

	extern ComPtr<D3D12MA::Allocator> g_Allocator;

	/// Debug helpers
#if defined (_DEBUG)
	extern ComPtr<ID3D12Debug5>			g_D3DDebug;
	extern ComPtr<ID3D12DebugDevice2>	g_DebugDevice;
	extern ComPtr<IDXGIDebug1>			g_DXGIDebug;
#endif

	extern D3D_FEATURE_LEVEL g_FeatureLevel;

}
