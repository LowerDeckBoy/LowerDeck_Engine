#include "D3D12Context.hpp"
#include "../Window/Window.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	std::unique_ptr<D3D12DescriptorHeap> D3D12Context::m_MainHeap = nullptr;
	std::unique_ptr<D3D12DescriptorHeap> D3D12Context::m_DepthHeap = nullptr;

	D3D12Context::FeatureSupport D3D12Context::FeatureSet = {};

	D3D12Context::~D3D12Context()
	{
		ReleaseD3D();
	}

	void D3D12Context::InitializeD3D()
	{
		if (IsInitialized())
		{
			// warning log
			//return false;
		}

		m_Viewport = new D3D12Viewport();
		m_Viewport->Set(Window::m_Resolution.Width, Window::m_Resolution.Height);

		D3D::InitializeDevice();
		D3D::InitializeCommands();
		D3D::InitializeSwapChain(*m_Viewport);

		InitializeHeaps();
		CheckFeatures();

		bInitialized = true;
	}

	void D3D12Context::InitializeHeaps()
	{
		// SRV
		m_MainHeap = std::make_unique<D3D12DescriptorHeap>(HeapUsage::eSRV_CBV_UAV, 4096, L"Main Descriptor Heap");

		// DSV
		m_DepthHeap = std::make_unique<D3D12DescriptorHeap>(HeapUsage::eDSV, 64, L"Main Depth Heap");
	}

	void D3D12Context::WaitForGPU()
	{
		ThrowIfFailed(g_CommandQueue.Get()->Signal(g_Fence.Get(), g_FenceValues.at(FRAME_INDEX)));

		ThrowIfFailed(g_Fence.Get()->SetEventOnCompletion(g_FenceValues.at(FRAME_INDEX), g_FenceEvent));
		::WaitForSingleObjectEx(g_FenceEvent, INFINITE, FALSE);

		g_FenceValues.at(FRAME_INDEX)++;
	}

	void D3D12Context::MoveToNextFrame()
	{
		const uint64_t currentFenceValue{ g_FenceValues.at(FRAME_INDEX) };
		ThrowIfFailed(g_CommandQueue->Signal(g_Fence.Get(), currentFenceValue));

		// Update the frame index.
		FRAME_INDEX = g_SwapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (g_Fence.Get()->GetCompletedValue() < g_FenceValues.at(FRAME_INDEX))
		{
			ThrowIfFailed(g_Fence.Get()->SetEventOnCompletion(g_FenceValues.at(FRAME_INDEX), g_FenceEvent));
			::WaitForSingleObjectEx(g_FenceEvent, INFINITE, FALSE);
		}

		g_FenceValues.at(FRAME_INDEX) = currentFenceValue + 1;
	}

	void D3D12Context::FlushGPU()
	{
		for (uint32_t i = 0; i < FRAME_COUNT; i++)
		{
			const uint64_t currentValue{ g_FenceValues.at(i) };

			ThrowIfFailed(g_CommandQueue->Signal(g_Fence.Get(), currentValue));
			g_FenceValues.at(i)++;

			if (g_Fence.Get()->GetCompletedValue() < currentValue)
			{
				ThrowIfFailed(g_Fence.Get()->SetEventOnCompletion(currentValue, g_FenceEvent));

				::WaitForSingleObject(g_FenceEvent, INFINITE);
			}
		}

		FRAME_INDEX = 0;
	}

	void D3D12Context::OnResize()
	{
		WaitForGPU();

		if (!g_Device.Get() || !g_SwapChain.Get() || !GetCommandAllocator())
			throw std::exception();

		ResetCommandLists();
		ReleaseRenderTargets();

		m_Viewport->Set(Window::m_Resolution.Width, Window::m_Resolution.Height);

		ThrowIfFailed(g_SwapChain.Get()->ResizeBuffers(
			FRAME_COUNT, 
			static_cast<uint32_t>(m_Viewport->Viewport().Width), static_cast<uint32_t>(m_Viewport->Viewport().Height),
			g_RenderTargetFormat, 0));

		FRAME_INDEX = 0;
		CreateBackbuffers();

		ExecuteCommandLists(false);
	}

	void D3D12Context::ReleaseD3D()
	{
		WaitForGPU();
		FlushGPU();

		// Despite being a std::unique_ptr Heap is released manualy.
		// Otherwise other structs are released before Heap causing to false-positive LIVE_DESCRIPTORHEAP.
		m_DepthHeap.reset();
		m_DepthHeap = nullptr;

		m_MainHeap.reset();
		m_MainHeap = nullptr;

		D3D::ReleaseSwapChain();
		D3D::ReleaseCommands();
		D3D::ReleaseDevice();
	}

	bool D3D12Context::IsInitialized()
	{
		return bInitialized;
	}

	void D3D12Context::ReleaseRenderTargets()
	{
		for (uint32_t i = 0; i < FRAME_COUNT; i++)
		{
			g_RenderTargets.at(i).Reset();
			g_FenceValues.at(i) = g_FenceValues.at(FRAME_INDEX);
		}
	}

	void D3D12Context::CheckFeatures()
	{
		auto checkShaderSupport = [&]() -> void {
			D3D12_FEATURE_DATA_SHADER_MODEL shaderModel{ D3D_SHADER_MODEL_6_6 };

			if (SUCCEEDED(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(D3D12_FEATURE_SHADER_MODEL))))
			{
#if defined(NTDDI_WIN10_VB) && (NTDDI_VERSION >= NTDDI_WIN10_VB)
				shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_6;
				debug::Print("HLSL Shader Model 6.6 is supported.\n");
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_6;
#elif defined(NTDDI_WIN10_19H1) && (NTDDI_VERSION >= NTDDI_WIN10_19H1)
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_5;
				debug::Print("Highest HLSL Shader: Model 6.5.\n");
#elif defined(NTDDI_WIN10_RS5) && (NTDDI_VERSION >= NTDDI_WIN10_RS5)
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_4;
				debug::Print("Highest HLSL Shader Model: 6.4.\n");
#elif defined(NTDDI_WIN10_RS4) && (NTDDI_VERSION >= NTDDI_WIN10_RS4)
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_2;
				debug::Print("Highest HLSL Shader: Model 6.2.\n");
#elif defined(NTDDI_WIN10_RS3) && (NTDDI_VERSION >= NTDDI_WIN10_RS3)
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_1;
				debug::Print("Highest HLSL Shader: Model 6.1.\n");
#else
				FeatureSet.ShaderModel = D3D_SHADER_MODEL_6_0;
				debug::Print("Highest HLSL Shader: Model 6.0.\n");
#endif
			}
			};
		checkShaderSupport();

		D3D12_FEATURE_DATA_D3D12_OPTIONS featuresSupport{};
			
		ThrowIfFailed(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featuresSupport, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS)), "Failed to enumarate GPU features!\n");
			
		// ResourceHeapTier
		{
			switch (featuresSupport.ResourceHeapTier)
			{
			case D3D12_RESOURCE_HEAP_TIER_2:
			{
				debug::Print("D3D12 Resource Heap Tier 2 is supported.\n");
				FeatureSet.HeapTier = D3D12_RESOURCE_HEAP_TIER_2;
				break;
			}
			case D3D12_RESOURCE_HEAP_TIER_1:
			{
				debug::Print("D3D12 Resource Heap Tier 1 is supported.\n");
				FeatureSet.HeapTier = D3D12_RESOURCE_HEAP_TIER_1;
				break;
			}
			}
		}

		// ResourceBindingTier
		{
			switch (featuresSupport.ResourceBindingTier)
			{
			case D3D12_RESOURCE_BINDING_TIER_3:
			{
				debug::Print("D3D12 Resource Binding Tier 3 is supported.\n");
				FeatureSet.BindingTier = D3D12_RESOURCE_BINDING_TIER_3;
				break;
			}
			case D3D12_RESOURCE_BINDING_TIER_2:
			{
				debug::Print("D3D12 Resource Binding Tier 2 is supported.\n");
				FeatureSet.BindingTier = D3D12_RESOURCE_BINDING_TIER_2;
				break;
			}
			case D3D12_RESOURCE_BINDING_TIER_1:
			{
				debug::Print("D3D12 Resource Binding Tier 1 is supported.\n");
				FeatureSet.BindingTier = D3D12_RESOURCE_BINDING_TIER_1;
				break;
			}
			}
		}

		// TiledResourcesTier
		{
			switch (featuresSupport.TiledResourcesTier)
			{
			case D3D12_TILED_RESOURCES_TIER_4:
			{
				debug::Print("D3D12 Tiled Resource Tier 4 is supported.\n");
				FeatureSet.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_4;
				break;
			}
			case D3D12_TILED_RESOURCES_TIER_3:
			{
				debug::Print("D3D12 Tiled Resource Tier 3 is supported.\n");
				FeatureSet.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_3;
				break;
			}
			case D3D12_TILED_RESOURCES_TIER_2:
			{
				debug::Print("D3D12 Tiled Resource Tier 2 is supported.\n");
				FeatureSet.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_2;
				break;
			}
			case D3D12_TILED_RESOURCES_TIER_1:
			{
				debug::Print("D3D12 Tiled Resource Tier 1 is supported.\n");
				FeatureSet.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_1;
				break;
			}
			default:
			{
				debug::Print("D3D12 Tiled Resource is NOT supported.\n");
				FeatureSet.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED;
				break;
			}
			}
		}

		D3D12_FEATURE_DATA_ARCHITECTURE1 dataArchitecture{};
		ThrowIfFailed(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &dataArchitecture, sizeof(dataArchitecture)));

		// Data Architecture
		{
			if (dataArchitecture.TileBasedRenderer == TRUE)
			{ 
				debug::Print("D3D12 Tile Based Renderer available.\n");
				FeatureSet.bTileBasedRenderer = TRUE;
			}
			else
			{
				debug::Print("D3D12 Tile Based Renderer NOT available.\n");
				FeatureSet.bTileBasedRenderer = FALSE;
			}

			if (dataArchitecture.UMA == TRUE)
			{
				debug::Print("D3D12 UMA available.\n");
				FeatureSet.bUMA = TRUE;
			}
			else
			{
				debug::Print("D3D12 UMA NOT available.\n");
				FeatureSet.bUMA = FALSE;
			}
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 featuresSupport5{};
		ThrowIfFailed(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featuresSupport5, sizeof(featuresSupport5)), "Failed to enumerate D3D12_FEATURE_DATA_D3D12_OPTIONS5!\n");

		// RenderPasses
		{
			switch (featuresSupport5.RenderPassesTier)
			{
			case D3D12_RENDER_PASS_TIER_2:
			{
				debug::Print("D3D12 Render Pass Tier 2 is supported.\n");
				FeatureSet.RenderPassTier = D3D12_RENDER_PASS_TIER_2;
				break;
			}
			case D3D12_RENDER_PASS_TIER_1:
			{
				debug::Print("D3D12 Render Pass Tier 1 is supported.\n");
				FeatureSet.RenderPassTier = D3D12_RENDER_PASS_TIER_1;
				break;
			}
			case D3D12_RENDER_PASS_TIER_0:
			{
				debug::Print("D3D12 Render Pass is NOT supported.\n");
				FeatureSet.RenderPassTier = D3D12_RENDER_PASS_TIER_0;
				break;
			}
			}
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS7 featuresSupport7{};
		ThrowIfFailed(g_Device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &featuresSupport7, sizeof(featuresSupport7)));

		// Mesh shaders
		{
			if (featuresSupport7.MeshShaderTier == D3D12_MESH_SHADER_TIER_1)
			{
				debug::Print("D3D12 Mesh Shader Tier 1 is supported.\n");
				FeatureSet.MeshShaderTier = D3D12_MESH_SHADER_TIER_1;
			}
			else
			{
				debug::Print("D3D12 Mesh Shader is NOT supported.\n");
				FeatureSet.MeshShaderTier = D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
			}
		}

		// TODO:
		// if raytracing D3D12_FEATURE_DATA_D3D12_OPTIONS3::BarycentricsSupported
	}
}
