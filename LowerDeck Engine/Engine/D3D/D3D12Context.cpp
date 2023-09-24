#include "D3D12Context.hpp"
#include "../Window/Window.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	std::unique_ptr<D3D12DescriptorHeap> D3D12Context::m_MainHeap = nullptr;

	D3D12Context::D3D12Context()
	{
	}

	D3D12Context::~D3D12Context()
	{
		ReleaseD3D();
	}

	bool D3D12Context::InitializeD3D()
	{
		if (IsInitialized())
		{
			// warning log
			//return false;
		}

		m_Viewport.Set(Window::m_Resolution.Width, Window::m_Resolution.Height);

		D3D::InitializeDevice();
		D3D::InitializeCommands();
		D3D::InitializeSwapChain(m_Viewport);

		InitializeHeaps();

		bInitialized = true;
		return true;
	}

	void D3D12Context::InitializeHeaps()
	{
		m_MainHeap = std::make_unique<D3D12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, L"Main Descriptor Heap");
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

		// Reset DepthStencil

		ThrowIfFailed(g_SwapChain.Get()->ResizeBuffers(
			FRAME_COUNT, 
			static_cast<uint32_t>(m_Viewport.Viewport().Width), static_cast<uint32_t>(m_Viewport.Viewport().Width), 
			g_RenderTargetFormat, 0));

		FRAME_INDEX = 0;

		// SetViewport();
		CreateBackbuffers();
		// DepthStencil

		ExecuteCommandLists(false);
	}

	void D3D12Context::ReleaseD3D()
	{
		WaitForGPU();
		FlushGPU();

		// Despite being a std::unique_ptr Heap is released manualy.
		// Otherwise other structs are released before Heap causing to false-positive LIVE_DESCRIPTORHEAP.
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
}