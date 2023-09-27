#pragma once
#include "../../D3D/D3D12Context.hpp"

// Likely to change namespace
namespace gfx
{
	/// <summary>
	/// Used to pass to Buffer upon creation.<br/>
	/// </summary>
	struct BufferData
	{
		BufferData() {}
		BufferData(void* pData, size_t Count, size_t Size, size_t Stride) :
			pData(pData), ElementsCount(static_cast<uint32_t>(Count)), Size(Size), Stride(static_cast<uint32_t>(Stride))
		{ }

		void* pData{ nullptr };
		uint32_t ElementsCount{ 0 };
		size_t	 Size{ 0 };
		uint32_t Stride{ 0 };
	};

	/// <summary>
	/// Indicating desired buffer type.<br/>
	/// Used to transit buffer resource into appropiate state.
	/// </summary>
	enum BufferType : uint8_t
	{
		eVertex		= 0x00,
		eIndex		= 0x01,
		eConstant	= 0x02,
		eStructured = 0x04
	};

	/// <summary>
	/// Base class for creating different buffer types.
	/// </summary>
	class Buffer
	{
	public:
		/// <summary></summary>
		Buffer() = default;
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="Usage"></param>
		/// <param name="bSRV"></param>
		Buffer(BufferData Data, BufferType Usage, bool bSRV = false);
		/// <summary>
		/// 
		/// </summary>
		~Buffer();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="Usage"></param>
		/// <param name="bSRV"></param>
		void Create(BufferData Data, BufferType Usage, bool bSRV = false);

		/// <summary>
		/// Note: Not sure if should be called in destructure rather then manually.
		/// Might cause issues when using DXR later.
		/// </summary>
		//virtual void Release() = 0;

		// TOOD: 
		// General helper function to map memory -> used when not making D3D12MA allocs.
		//static void MapMemory();

		/// <summary></summary>
		/// <returns></returns>
		inline ID3D12Resource* GetBuffer() const noexcept { return m_Buffer.Get(); }
		/// <summary></summary>
		/// <returns></returns>
		inline const D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Buffer.Get()->GetGPUVirtualAddress(); }

		/// <summary></summary>
		/// <returns></returns>
		inline BufferData GetData() noexcept { return m_BufferData; }


	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		D3D12MA::Allocation* m_Allocation{ nullptr };

		D3D::D3D12Descriptor m_Descriptor;

		BufferData m_BufferData{};
	};

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pResource"></param>
	/// <param name="Data"></param>
	/// <param name="Descriptor"></param>
	static void CreateSRV(ID3D12Resource* pResource, BufferData Data, D3D::D3D12Descriptor& Descriptor);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pResource"></param>
	/// <param name="Data"></param>
	/// <param name="Descriptor"></param>
	static void CreateUAV(ID3D12Resource* pResource, BufferData Data, D3D::D3D12Descriptor& Descriptor);
}
