#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class D3D12Descriptor;

namespace D3D
{
	enum class HeapUsage : uint8_t
	{
		eShader = 0x00,
		eRTV,
		eDSV,
		eSampler
	};

	/// <summary>
	/// Unfinished solution
	/// </summary>
	class D3D12DescriptorHeap
	{
	public:
		/// <summary>
		/// Desc based constructor.
		/// </summary>
		/// <param name="Desc"> Heap Desc. </param>
		/// <param name="DebugName"> Optional. </param>
		D3D12DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& Desc, const LPCWSTR& DebugName = L"");
		/// <summary>
		/// 
		/// </summary>
		/// <param name="eUsage"> Type of <c>Descriptors</c>. </param>
		/// <param name="MaxCount"> Maximum capacity of the Heap. </param>
		/// <param name="DebugName"> Optional. </param>
		D3D12DescriptorHeap(HeapUsage eUsage, uint32_t MaxCount, const LPCWSTR& DebugName = L"");
		D3D12DescriptorHeap(const D3D12DescriptorHeap&) = delete;
		D3D12DescriptorHeap(const D3D12DescriptorHeap&&) = delete;
		D3D12DescriptorHeap operator=(const D3D12DescriptorHeap&) = delete;
		~D3D12DescriptorHeap();

		/// <summary>
		/// Allocate given <b><c>D3D12Descriptor</c></b> inside the Heap of this class object.
		/// </summary>
		/// <param name="TargetDescriptor"> <c>Descriptor</c> to allocate. </param>
		/// <param name="Count"> Amount of descriptors to allocate. </param>
		void Allocate(D3D12Descriptor& TargetDescriptor, uint32_t Count = 1);
		
		/// <summary>
		/// Used when a <b>D3D12Descriptor</b> is already allocated (non-zero pointer)
		/// to avoid allocating another, redundant index inside of the Heap.
		/// </summary>
		/// <param name="TargetDescriptor"> <c>Descriptor</c> to override. </param>
		/// <param name="Count"> Amount of descriptors to override. </param>
		void Override(D3D12Descriptor& TargetDescriptor, uint32_t Count);
		
		/// <summary> Getter for underlaying ID3D12DescriptorHeap COM pointer. </summary>
		/// <returns> ID3D12DescriptorHeap* </returns>
		[[nodiscard]] inline ID3D12DescriptorHeap*  Heap() { return m_Heap.Get(); }
		/// <summary>  </summary>
		/// <returns> <c>GetAddressOf()</c> of underlying Heap. </returns>
		[[nodiscard]] inline ID3D12DescriptorHeap** HeapAddressOf() { return m_Heap.GetAddressOf(); }

		/// <summary> Gets Heap CPU start address. </summary>
		/// <returns> Address to the beginning of a Heap. </returns>
		[[nodiscard]] inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return m_Heap.Get()->GetCPUDescriptorHandleForHeapStart(); }
		/// <summary> Gets Heap GPU start address. </summary>
		/// <returns> Address to the beginning of a Heap. </returns>
		[[nodiscard]] inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return m_Heap.Get()->GetGPUDescriptorHandleForHeapStart(); }

		[[nodiscard]] inline uint32_t GetDescriptorSize() const { return m_DescriptorSize; }

		/// <summary></summary>
		/// <returns> CPU Handle of given index from the Heap. </returns>
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUptr(uint32_t Count);
		/// <summary></summary>
		/// <returns> GPU Handle of given index from the Heap. </returns>
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUptr(uint32_t Count);

		void Release();

	private:
		/// <summary> Actual Heap. </summary>
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		/// <summary>
		/// Size of single allocation.<br/>
		/// Defaults to 32 but size is checked during Heap creation anyway.
		/// </summary>
		uint32_t m_DescriptorSize{ 32 };
		/// <summary>
		/// Max count of possible allocations in this Heap.
		/// </summary>
		uint32_t m_MaxDescriptors{ 0 };
		/// <summary>
		/// How many allocation can still be done in this Heap.
		/// </summary>
		uint32_t m_AvailableAllocs{ 0 };
		
		/// <summary>
		/// Retrieves beginning of the Heap for CPU handles.
		/// </summary>
		/// <returns> CPU beginning of the Heap. </returns>
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHeapStart() { return m_Heap.Get()->GetCPUDescriptorHandleForHeapStart(); }
		/// <summary>
		/// Retrieves beginning of the Heap for GPU handles.
		/// </summary>
		/// <returns> GPU beginning of the Heap. </returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GpuHeapStart() { return m_Heap.Get()->GetGPUDescriptorHandleForHeapStart(); }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="Offset"></param>
		/// <returns></returns>
		size_t GetHandleFromOffset(uint32_t Offset);

		/// <summary>
		///	Pointer to next available CPU handle for allocation.
		/// </summary>
		uint64_t m_AvailableCpuPtr{ 0 };
		/// <summary>
		/// Pointer to next available GPU handle for allocation.
		/// </summary>
		uint64_t m_AvailableGpuPtr{ 0 };

		/// <summary>
		/// Indicates index of a Descriptor inside this Heap.
		/// </summary>
		/// <returns> Index of a Heap allocation. </returns>
		uint32_t GetIndex(D3D12Descriptor& TargetDescriptor);

		/// <summary>
		/// Can allocate only when there are available allocation.
		/// </summary>
		/// <returns> <b>True</b> is allocation is possible. </returns>
		inline bool CanAllocate() const { return (m_AvailableAllocs > 0); }
	};
}
