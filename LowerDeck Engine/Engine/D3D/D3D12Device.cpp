#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	ComPtr<IDXGIFactory7> g_Factory{ nullptr };
	ComPtr<IDXGIAdapter3> g_Adapter{ nullptr };
	ComPtr<ID3D12Device5> g_Device { nullptr };

	ComPtr<D3D12MA::Allocator> g_Allocator{ nullptr };

#if defined (_DEBUG)
	ComPtr<ID3D12Debug5>		g_D3DDebug	 { nullptr };
	ComPtr<ID3D12DebugDevice2>	g_DebugDevice{ nullptr };
	ComPtr<IDXGIDebug1>			g_DXGIDebug	 { nullptr };
#endif 

	D3D_FEATURE_LEVEL g_FeatureLevel = D3D_FEATURE_LEVEL_12_2;

	bool InitializeDevice()
	{
		uint32_t dxgiFactoryFlags = 0;
#if defined (_DEBUG)
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(g_D3DDebug.ReleaseAndGetAddressOf())), "Failed to create Debug Interface!");
		g_D3DDebug.Get()->EnableDebugLayer();
		g_D3DDebug.Get()->SetEnableAutoName(TRUE);
#endif

		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(g_Factory.ReleaseAndGetAddressOf())), "Failed to CreateDXGIFactory2!");

		ComPtr<IDXGIAdapter1> adapter;
		g_Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));

		if (FAILED(D3D12CreateDevice(adapter.Get(), g_FeatureLevel, __uuidof(ID3D12Device), nullptr)))
		{
			ReleaseDevice();
			utility::ErrorMessage("Failed to find suitable GPU!");
			std::exit(EXIT_FAILURE);
		}

		ComPtr<ID3D12Device> device;
		ThrowIfFailed(D3D12CreateDevice(adapter.Get(), g_FeatureLevel, IID_PPV_ARGS(device.ReleaseAndGetAddressOf())),
			"Failed to create D3D Device!");

		ThrowIfFailed(device.As(&g_Device));
		SAFE_RELEASE(device);

		adapter.As(&g_Adapter);
		SAFE_RELEASE(adapter);

		CheckRaytracingSupport();

#if defined (_DEBUG)
		ThrowIfFailed(g_Device.Get()->QueryInterface(g_DebugDevice.GetAddressOf()), "Failed to Query D3D Debug Device!");
		ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&g_DXGIDebug)), "Failed to get DXGI Debug Interface!");
#endif

		D3D12MA::ALLOCATOR_DESC allocatorDesc{};
		allocatorDesc.pDevice  = g_Device.Get();
		allocatorDesc.pAdapter = g_Adapter.Get();
		ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, g_Allocator.ReleaseAndGetAddressOf()));

		return true;
	}

	bool CheckRaytracingSupport()
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 features{};
		if (SUCCEEDED(D3D12CreateDevice(g_Adapter.Get(), g_FeatureLevel, IID_PPV_ARGS(g_Device.GetAddressOf()))))
		{
			if (SUCCEEDED(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features))))
			{
				if (features.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
				{
					DXGI_ADAPTER_DESC1 desc{};
					g_Adapter.Get()->GetDesc1(&desc);
					const auto& message{ std::wstring(desc.Description) + L" does NOT provide raytracing support!" };

					utility::ErrorMessage(message);
					ReleaseDevice();
					std::exit(EXIT_FAILURE);
				}
			}
		}

		return true;
	}

	uint32_t QueryAdapterMemory()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo{};
		g_Adapter.Get()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

		return static_cast<uint32_t>(memoryInfo.CurrentUsage / 1024 / 1024);
	}

	void HeapAllocation(ID3D12Resource** ppResource, 
		D3D12MA::Allocation** ppAllocation, 
		const CD3DX12_RESOURCE_DESC& HeapDesc, 
		D3D12MA::ALLOCATION_FLAGS AllocationFlags,
		D3D12_HEAP_TYPE HeapType,
		D3D12_HEAP_FLAGS HeapFlags)
	{
		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = HeapType;
		allocDesc.Flags = AllocationFlags;
		allocDesc.ExtraHeapFlags = HeapFlags;

		D3D12_RESOURCE_STATES state{ D3D12_RESOURCE_STATE_GENERIC_READ };

		if (HeapType == D3D12_HEAP_TYPE_DEFAULT)
			state = D3D12_RESOURCE_STATE_COPY_DEST;

		ThrowIfFailed(D3D::g_Allocator->CreateResource(&allocDesc, &HeapDesc, state, nullptr, ppAllocation, IID_PPV_ARGS(ppResource)));

	}

	void ReleaseDevice()
	{
		SAFE_RELEASE(g_Allocator);

		SAFE_RELEASE(g_Device);
		SAFE_RELEASE(g_Adapter);
		SAFE_RELEASE(g_Factory);
		
		// Release Debug Interfaces at the end
		// otherwise some output information might be false-positive live producers/objects
#if defined (_DEBUG)
		//g_DebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY);
		SAFE_RELEASE(g_DebugDevice);
		SAFE_RELEASE(g_D3DDebug);
		g_DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		SAFE_RELEASE(g_DXGIDebug);
#endif
	}
}
