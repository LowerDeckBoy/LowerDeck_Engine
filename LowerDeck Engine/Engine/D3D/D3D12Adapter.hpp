#pragma once
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>

using Microsoft::WRL::ComPtr;

#define GPU_VENDOR_NVIDIA			0x10DE
#define GPU_VENDOR_AMD				0x1002
#define GPU_VENDOR_INTEL			0x8086

// TODO:
#define VENDOR_ARCH_NVIDIA_PASCAL	0x1000
#define VENDOR_ARCH_NVIDIA_VOLTA	0x1001
#define VENDOR_ARCH_NVIDIA_TURING	0x1002
#define VENDOR_ARCH_NVIDIA_AMPERE	0x1004
#define VENDOR_ARCH_NVIDIA_ADA		0x1008

#define VENDOR_ARCH_AMD_GCN_1		0x2010
#define VENDOR_ARCH_AMD_GCN_2		0x2020
#define VENDOR_ARCH_AMD_GCN_3		0x2030
#define VENDOR_ARCH_AMD_GCN_4		0x2040
#define VENDOR_ARCH_AMD_GCN_5		0x2050
#define VENDOR_ARCH_AMD_RDNA_1		0x2110
#define VENDOR_ARCH_AMD_RDNA_2		0x2120
#define VENDOR_ARCH_AMD_RDNA_3		0x2130

#define VENDOR_ARCH_INTEL			0x3000
#define VENDOR_ARCH_INTEL_ARC		0x3010

namespace D3D
{
	class D3D12Adapter
	{
	public:
		ComPtr<IDXGIFactory7> Factory;
		ComPtr<IDXGIAdapter4> Adapter;


	private:
		

	};

	/// <summary>
	/// 
	/// </summary>
	struct D3D12AdapterMemory
	{
		
		uint64_t AvailableMemory;
		uint64_t TotalMemory;
	};
}
