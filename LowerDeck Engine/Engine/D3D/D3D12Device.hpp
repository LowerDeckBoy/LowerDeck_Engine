#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
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
// whenever D3D context is required, via using global variables.
namespace D3D
{
	bool InitializeDevice();
	/// Raytracing support is mendatory
	/// as engine is meant to be built around hybrid path
	bool CheckRaytracingSupport();
	/// <summary> GPU Virtual Memory usage. </summary>
	/// <returns> Current VRAM usage in MegaBytes. </returns>
	uint32_t QueryAdapterMemory();
	/// Release all Device related ComPtrs
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
