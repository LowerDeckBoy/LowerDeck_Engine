#include "D3D12Command.hpp"
#include "D3D12Device.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Types.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12PipelineState.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
    // Define all extern pointers
    ComPtr<ID3D12GraphicsCommandList4>	g_CommandList        { nullptr };
    ComPtr<ID3D12CommandQueue>			g_CommandQueue       { nullptr };
    ComPtr<ID3D12CommandQueue>			g_CommandComputeQueue{ nullptr };

    std::array<ComPtr<ID3D12CommandAllocator>, FRAME_COUNT> g_CommandAllocators;

    void InitializeCommands()
    {
        // Create ID3D12CommandAllocator for each frame
        for (uint32_t i = 0; i < FRAME_COUNT; i++)
        {
            ThrowIfFailed(g_Device.Get()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(g_CommandAllocators.at(i).ReleaseAndGetAddressOf())),
                "Failed to create ID3D12CommandAllocators!");

            std::wstring debugName{ L"Command Allocator #" + std::to_wstring(i) };
            g_CommandAllocators.at(i).Get()->SetName(debugName.c_str());
        }

        ThrowIfFailed(g_Device.Get()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            g_CommandAllocators.at(0).Get(), nullptr, IID_PPV_ARGS(g_CommandList.ReleaseAndGetAddressOf())),
            "Failed to create ID3D12GraphicsCommandList4!");
        g_CommandList.Get()->SetName(L"Graphics Command List");

        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Priority = 0;
        ThrowIfFailed(g_Device.Get()->CreateCommandQueue(&desc, IID_PPV_ARGS(g_CommandQueue.ReleaseAndGetAddressOf())),
            "Failed to create Command Queue!");
        g_CommandQueue.Get()->SetName(L"Direct Command Queue");

        desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        ThrowIfFailed(g_Device.Get()->CreateCommandQueue(&desc, IID_PPV_ARGS(g_CommandComputeQueue.ReleaseAndGetAddressOf())), "Failed to create Command Compute Queue!");
        g_CommandComputeQueue.Get()->SetName(L"Compute Command Queue");

    }

    void WaitForGPU()
    {
        ThrowIfFailed(g_CommandQueue.Get()->Signal(g_Fence.Get(), g_FenceValues.at(FRAME_INDEX)));

        ThrowIfFailed(g_Fence.Get()->SetEventOnCompletion(g_FenceValues.at(FRAME_INDEX), g_FenceEvent));
        ::WaitForSingleObjectEx(g_FenceEvent, INFINITE, FALSE);

        g_FenceValues.at(FRAME_INDEX)++;
    }

    void FlushGPU()
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

    void ExecuteCommandLists(bool bResetAllocator)
    {
        ThrowIfFailed(g_CommandList.Get()->Close(), "Failed to close ID3D12GraphicsCommandList!");

        std::array<ID3D12CommandList*, 1> pCommandLists{ g_CommandList.Get() };
        g_CommandQueue.Get()->ExecuteCommandLists(static_cast<uint32_t>(pCommandLists.size()), pCommandLists.data());
    
        if (bResetAllocator)
            ThrowIfFailed(g_CommandList.Get()->Reset(GetCommandAllocator(), nullptr));

        D3D::WaitForGPU();
    }

    void ResetCommandLists()
    {
        ThrowIfFailed(GetCommandAllocator()->Reset(), "Failed to Reset Command Allocator!");
        ThrowIfFailed(g_CommandList.Get()->Reset(GetCommandAllocator(), nullptr), "Failed to Reset Command List!");
    }

    void SetRootSignature(D3D12RootSignature& RootSignature, PipelineType Usage)
    {
        if (Usage == PipelineType::eGraphics)
            g_CommandList.Get()->SetGraphicsRootSignature(RootSignature.Get());
        else
            g_CommandList.Get()->SetComputeRootSignature(RootSignature.Get());
    }

    void SetPSO(D3D12PipelineState& PSO)
    {
        g_CommandList.Get()->SetPipelineState(PSO.Get());
    }

    void TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
    {
        const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(pResource, Before, After) };
        g_CommandList.Get()->ResourceBarrier(1, &barrier);
    }

    void ReleaseCommands()
    {
        for (auto& allocator : g_CommandAllocators)
            SAFE_RELEASE(allocator);

        SAFE_RELEASE(g_CommandComputeQueue);
        SAFE_RELEASE(g_CommandQueue);
        SAFE_RELEASE(g_CommandList);
    }
}
