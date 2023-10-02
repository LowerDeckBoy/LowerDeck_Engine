#pragma once
#include "D3D12Device.hpp"
#include "D3D12Viewport.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Command.hpp"
#include "D3D12Utility.hpp"

namespace D3D
{
	/// <summary>
	/// Main class (entry point) for DirectX 12 based context.<br/>
	/// All pointers to DX structures are initialized here,
	/// as well as they are cleanup here via class destructor.<br/>
	/// Class is meant for simplying and securing both context creation and it's later cleanup.
	/// </summary>
	class D3D12Context
	{
	public:
		D3D12Context() = default;
		D3D12Context(const D3D12Context&) = delete;
		D3D12Context(const D3D12Context&&) = delete;
		D3D12Context operator=(const D3D12Context&) = delete;
		~D3D12Context();

		bool InitializeD3D();
		bool InitializeD3D(uint32_t Width, uint32_t Height);

		void InitializeHeaps();

		/// <summary>
		/// Wait until <c>Fence</c> event is complete.
		/// </summary>
		void WaitForGPU();
		/// <summary>
		/// Move to next <c>SwapChain Backbuffer</c><br/>.
		/// If frame is not ready to be presented, wait until it is.
		/// </summary>
		void MoveToNextFrame();
		/// <summary>
		/// Clear <c>Fence Event values</c> then move back to first backbuffer.
		/// </summary>
		void FlushGPU();

		/// <summary>
		/// Resize <c>Scene Viewport</c> and resize all <c>SwapChain Backbuffers</c>.
		/// </summary>
		void OnResize();

		/// <summary>
		/// Safe release of all member <c>ComPtrs</c> and other DirectX based structs.<br/>
		/// Note: If in Debug mode, check for live objects.
		/// </summary>
		void ReleaseD3D();

		/// <summary>
		/// Preventing from re-initializing D3D context.
		/// </summary>
		/// <returns> <b>True</b> if was already initialized. </returns>
		bool IsInitialized();

		/// <summary>
		/// 
		/// </summary>
		/// <returns> Pointer to SRV/CBV/UAV Descriptor Heap. </returns>
		[[nodiscard]] inline static D3D12DescriptorHeap* GetMainHeap() { return m_MainHeap.get(); }

		/// <summary>
		/// 
		/// </summary>
		/// <returns> Pointer to general use DSV Descriptor Heap. </returns>
		[[nodiscard]] inline static D3D12DescriptorHeap* GetDepthHeap() { return m_DepthHeap.get(); }

		/// <summary>
		/// 
		/// </summary>
		/// <returns> Pointer to Scene Viewport. </returns>
		D3D12Viewport* GetSceneViewport() const { return m_Viewport; }

	protected:
		D3D12Viewport* m_Viewport{ nullptr };

		bool bInitialized{ false };

	private:
		/// <summary>
		/// Used before resizing SwapChain context.
		/// Required to proper resizing of SwapChain Backbuffers.
		/// </summary>
		void ReleaseRenderTargets();

		// TODO:
		// https://github.com/microsoft/DirectXTK12/wiki/Shader-Model-6
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_d3d12_options
		/// <summary>
		/// Query GPU capabilities.<br/>
		/// Validates if current GPU does provide necessary features to run the engine.<br/>
		/// Features that must be supported:<br/>
		/// - Shader Model 6.0+. Desired 6.6 support, but 6.3 will suffice for current needs.<br/>
		/// - Raytracing support.<br/>
		/// </summary>
		void CheckFeatures();

		/// <summary>
		/// CBV_SRV_UAV heap.<br/>Main source of resource allocations.
		/// </summary>
		static std::unique_ptr<D3D12DescriptorHeap> m_MainHeap;
		/// <summary>
		/// DSV heap.
		/// </summary>
		static std::unique_ptr<D3D12DescriptorHeap> m_DepthHeap;

		/// <summary>
		/// Supported and desired Device features
		/// </summary>
		struct SupportedFeatures
		{
			/// <summary>
			/// Highest supported shader model.<br/>
			/// Desired level: 6.6.
			/// </summary>
			D3D_SHADER_MODEL ShaderModel;
			/// <summary>
			/// 
			/// </summary>
			D3D12_MESH_SHADER_TIER MeshShaderTier;
			/// <summary>
			/// 
			/// </summary>
			D3D12_RESOURCE_HEAP_TIER HeapTier;
			/// <summary>
			/// 
			/// </summary>
			D3D12_RESOURCE_BINDING_TIER BindingTier;
			/// <summary>
			/// 
			/// </summary>
			D3D12_TILED_RESOURCES_TIER TiledResourcesTier;
			/// <summary>
			/// 
			/// </summary>
			D3D12_RENDER_PASS_TIER RenderPassTier;
			/// <summary>
			/// 
			/// </summary>
			BOOL bTileBasedRenderer;
			/// <summary>
			/// 
			/// </summary>
			BOOL bUMA;

			DXGI_FORMAT DisplayFormat;
		};
		
		static SupportedFeatures FeatureSet;

	};
}


