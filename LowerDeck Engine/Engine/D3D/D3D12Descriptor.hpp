#pragma once
#include <d3d12.h>

namespace D3D
{
	/// <summary> 
	/// D3D12Descriptor takes a role of a wrapper around single descriptor object.<br/>
	/// Holds pointer to allocated CPU handle address and (for SRV/CBV/UAV heap type) GPU pointer. 
	/// </summary>
	class D3D12Descriptor
	{
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPU;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPU;

	public:
		~D3D12Descriptor()
		{
			m_CPU.ptr = 0;
			m_GPU.ptr = 0;
		}

		/// Setter for CPU descriptor handle
		inline void SetCPU(D3D12_CPU_DESCRIPTOR_HANDLE Handle) noexcept { m_CPU = Handle; }
		/// Setter for GPU descriptor handle
		inline void SetGPU(D3D12_GPU_DESCRIPTOR_HANDLE Handle) noexcept { m_GPU = Handle; }

		/// Get CPU descriptor handle
		[[nodiscard]] inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const noexcept { return m_CPU; }

		/// Get GPU descriptor handle
		[[nodiscard]] inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const noexcept { return m_GPU; }

		/// <summary> Indicates whether Descriptor is allocated. </summary>
		/// <returns> True if is Descriptor's CPU pointer is non-zero. </returns>
		constexpr inline bool IsValid() const { return m_CPU.ptr != 0; }

		unsigned int Index{};
	};
}
