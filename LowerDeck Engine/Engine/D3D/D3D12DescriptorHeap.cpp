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
		m_AvailableAllocs = m_MaxDescriptors;

		m_AvailableCpuPtr = static_cast<uint64_t>(m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
		if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			m_AvailableGpuPtr = static_cast<uint64_t>(m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(HeapUsage Usage, uint32_t MaxCount, const LPCWSTR& DebugName)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = MaxCount;

		switch (Usage)
		{
		case HeapUsage::eShader:
		{
			desc.Type	= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags	= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			break;
		}
		case HeapUsage::eRTV:
		{
			desc.Type	= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.Flags	= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			break;
		}
		case HeapUsage::eDSV:
		{
			desc.Type	= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			desc.Flags	= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			break;
		}
		}

		ThrowIfFailed(g_Device.Get()->CreateDescriptorHeap(
			&desc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())),
			"Failed to create ID3D12DescriptorHeap!\n");
		if (DebugName)
			m_Heap.Get()->SetName(DebugName);

		m_Type = desc.Type;
		m_DescriptorSize = g_Device.Get()->GetDescriptorHandleIncrementSize(desc.Type);
		m_MaxDescriptors = desc.NumDescriptors;
		m_AvailableAllocs = m_MaxDescriptors;

		m_AvailableCpuPtr = static_cast<uint64_t>(m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
		if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			m_AvailableGpuPtr = static_cast<uint64_t>(m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		Release();
	}

	void D3D12DescriptorHeap::Allocate(D3D12Descriptor& TargetDescriptor, uint32_t Count)
	{
		if (!CanAllocate())
		{
			// Should throw warning for now
		}

		if (!TargetDescriptor.IsValid())
		{
			TargetDescriptor.SetCPU(GetCPUptr(Count));

			// Types D3D12_DESCRIPTOR_HEAP_TYPE_DSV and D3D12_DESCRIPTOR_HEAP_TYPE_RTV
			// cannot allocate GPU Handle, thus we skip allocating them to prevent causing potential errors.
			if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
				TargetDescriptor.SetGPU(GetGPUptr(Count));

			TargetDescriptor.Index = GetIndex(TargetDescriptor);
			m_AvailableAllocs -= Count;
		}
		else
		{
			Override(TargetDescriptor, Count);
		}

 	}
	
	void D3D12DescriptorHeap::Override(D3D12Descriptor& TargetDescriptor, uint32_t Count)
	{
		auto offset = GetIndex(TargetDescriptor);
		D3D12_CPU_DESCRIPTOR_HANDLE outputc = (D3D12_CPU_DESCRIPTOR_HANDLE)(CpuHeapStart().ptr + (offset * m_DescriptorSize));
		TargetDescriptor.SetCPU(outputc);
		if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE outputg = (D3D12_GPU_DESCRIPTOR_HANDLE)(GpuHeapStart().ptr + (offset * m_DescriptorSize));
			TargetDescriptor.SetGPU(outputg);
		}

		//TargetDescriptor.SetCPU(GetCPUptr(TargetDescriptor.Index, Count));
		//if (m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV && m_Type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		//	TargetDescriptor.SetGPU(GetGPUptr(TargetDescriptor.Index, Count));
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCPUptr(uint32_t Count)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE output = (D3D12_CPU_DESCRIPTOR_HANDLE)(m_AvailableCpuPtr);
		m_AvailableCpuPtr += ((size_t)Count * m_DescriptorSize);
		return output ;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGPUptr(uint32_t Count)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE output = (D3D12_GPU_DESCRIPTOR_HANDLE)(m_AvailableGpuPtr);
		m_AvailableGpuPtr += ((size_t)Count * m_DescriptorSize);
		return output;
	}

	size_t D3D12DescriptorHeap::GetHandleFromOffset(uint32_t Offset)
	{
		return size_t();
	}

	uint32_t D3D12DescriptorHeap::GetIndex(D3D12Descriptor& TargetDescriptor)
	{
		return static_cast<uint32_t>((TargetDescriptor.GetCPU().ptr - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize);
	}

	void D3D12DescriptorHeap::Release()
	{
		SAFE_RELEASE(m_Heap);
	}

}
