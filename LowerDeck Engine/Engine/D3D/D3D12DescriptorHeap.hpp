#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class D3D12Descriptor;

namespace D3D
{
	/// <summary>
	/// Unfinished solution
	/// </summary>
	class D3D12DescriptorHeap
	{
	public:
		D3D12DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& Desc, const LPCWSTR& DebugName = L"");
		D3D12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount, const LPCWSTR& DebugName = L"");
		D3D12DescriptorHeap(const D3D12DescriptorHeap&) = delete;
		D3D12DescriptorHeap(const D3D12DescriptorHeap&&) = delete;
		D3D12DescriptorHeap operator=(const D3D12DescriptorHeap&) = delete;
		~D3D12DescriptorHeap();

		/// <summary>
		/// Allocate given D3D12Descriptor inside the Heap of this class object.
		/// </summary>
		/// <param name="TargetDescriptor"></param>
		void Allocate(D3D12Descriptor& TargetDescriptor);
		
		/// <summary>
		/// Used when a D3D12Descriptor is already allocated (non-zero pointer)
		/// to avoid allocating another, redundant index inside of the Heap.
		/// </summary>
		/// <param name="TargetDescriptor"></param>
		void Override(D3D12Descriptor& TargetDescriptor);
		
		/// <summary> Getter for underlaying ID3D12DescriptorHeap COM pointer. </summary>
		/// <returns> ID3D12DescriptorHeap* </returns>
		[[nodiscard]] inline ID3D12DescriptorHeap* Heap() { return m_Heap.Get(); }

		[[nodiscard]] inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_Heap.Get()->GetCPUDescriptorHandleForHeapStart(); }
		[[nodiscard]] inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_Heap.Get()->GetGPUDescriptorHandleForHeapStart(); }

		[[nodiscard]] inline uint32_t GetDescriptorSize() const { return m_DescriptorSize; }

		/// <summary></summary>
		/// <returns> CPU Handle of given index from the Heap. </returns>
		[[maybe_unused]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUptr(uint32_t Index) { return { m_Heap.Get()->GetCPUDescriptorHandleForHeapStart().ptr + (size_t)Index * m_DescriptorSize }; }
		/// <summary></summary>
		/// <returns> GPU Handle of given index from the Heap. </returns>
		[[maybe_unused]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUptr(uint32_t Index) { return { m_Heap.Get()->GetGPUDescriptorHandleForHeapStart().ptr + (size_t)Index * m_DescriptorSize }; }

		void Release();

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		uint32_t m_DescriptorSize{ sizeof(m_Type) };
		uint32_t m_MaxDescriptors{ 0 };
		uint32_t m_Allocated{ 0 };

		inline bool CanAllocate() const { return (m_Allocated < m_MaxDescriptors); }
	};
}
