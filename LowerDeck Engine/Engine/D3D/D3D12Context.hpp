#pragma once
#include "D3D12Device.hpp"
#include "D3D12Viewport.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Command.hpp"
#include "D3D12Utility.hpp"

namespace D3D
{
	/// <summary>
	/// Main class (entry point) for DirectX 12 based context.
	/// All pointers to DX structures are initialized here,
	/// as well as they are cleanup here via class destructor.
	/// It is meant for simplying and securing both context creation and it's later cleanup.
	/// </summary>
	class D3D12Context
	{
	public:
		D3D12Context();
		D3D12Context(const D3D12Context&) = delete;
		D3D12Context(const D3D12Context&&) = delete;
		D3D12Context operator=(const D3D12Context&) = delete;
		~D3D12Context();

		bool InitializeD3D();

		void InitializeHeaps();

		void WaitForGPU();
		void MoveToNextFrame();
		void FlushGPU();

		void OnResize();

		void ReleaseD3D();

		bool IsInitialized();

		inline static D3D12DescriptorHeap* GetMainHeap() { return m_MainHeap.get(); }

		const D3D12Viewport& GetSceneViewport() const { return m_Viewport; }

	protected:
		D3D12Viewport m_Viewport;

		bool bInitialized{ false };

	public:
		// CBV_SRV_UAV heap
		// Main source of resource allocations
		static std::unique_ptr<D3D12DescriptorHeap> m_MainHeap;

	};
}


