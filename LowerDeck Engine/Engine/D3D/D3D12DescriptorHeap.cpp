#include "D3D12Descriptor.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& Desc, const LPCWSTR& DebugName)
	{
		ThrowIfFailed(g_Device.Get()->CreateDescriptorHeap(
			&Desc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())),
			"Failed to create ID3D12DescriptorHeap!\n");

		if (DebugName)
			m_Heap.Get()->SetName(DebugName);

		m_Type			 = Desc.Type;
		m_DescriptorSize = sizeof(Desc.Type);
		m_MaxDescriptors = Desc.NumDescriptors;
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount, const LPCWSTR& DebugName)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = MaxCount;
		desc.Type = Type;
		if (Type & D3D12_DESCRIPTOR_HEAP_TYPE_RTV || Type & D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		else
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ThrowIfFailed(g_Device.Get()->CreateDescriptorHeap(
			&desc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())),
			"Failed to create ID3D12DescriptorHeap!\n");
		if (DebugName)
			m_Heap.Get()->SetName(DebugName);

		m_Type = desc.Type;
		m_DescriptorSize = g_Device.Get()->GetDescriptorHandleIncrementSize(Type);
		m_MaxDescriptors = desc.NumDescriptors;
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		Release();
	}

	void D3D12DescriptorHeap::Allocate(D3D12Descriptor& TargetDescriptor)
	{
		if (!CanAllocate())
		{
			// Should throw error for now
		}

		if (!TargetDescriptor.IsValid())
		{
			++m_Allocated;
			TargetDescriptor.SetCPU(GetCPUptr(m_Allocated));
			// Types D3D12_DESCRIPTOR_HEAP_TYPE_DSV and D3D12_DESCRIPTOR_HEAP_TYPE_RTV
			// cannot allocate GPU Handle, thus we skip allocating them to prevent causing potential errors.
			if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
				TargetDescriptor.SetGPU(GetGPUptr(m_Allocated));

			TargetDescriptor.Index = m_Allocated;
		}
		else
		{
			Override(TargetDescriptor);
		}
	}

	void D3D12DescriptorHeap::Override(D3D12Descriptor& TargetDescriptor)
	{
		TargetDescriptor.SetCPU(GetCPUptr(TargetDescriptor.Index));
		TargetDescriptor.SetGPU(GetGPUptr(TargetDescriptor.Index));
	}

	void D3D12DescriptorHeap::Release()
	{
		SAFE_RELEASE(m_Heap);
	}
}
