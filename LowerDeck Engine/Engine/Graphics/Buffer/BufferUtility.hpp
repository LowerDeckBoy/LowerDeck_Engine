#pragma once
#include "../../D3D/D3D12Context.hpp"

// Likely to change namespace
namespace gfx
{
	/// <summary>
	/// 
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

	struct BufferDesc
	{
		CD3DX12_HEAP_PROPERTIES HeapProperties{ D3D12_HEAP_TYPE_UPLOAD };
		D3D12_RESOURCE_STATES	State{ D3D12_RESOURCE_STATE_GENERIC_READ };
		D3D12_HEAP_FLAGS		HeapFlags{ D3D12_HEAP_FLAG_NONE };
		DXGI_FORMAT				Format{ DXGI_FORMAT_UNKNOWN };
	};

	enum BufferType
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
		Buffer() = default;
		Buffer(BufferData Data, BufferType TypeOf, bool bSRV);
		~Buffer();

		void Create(BufferData Data, BufferType TypeOf, bool bSRV);

		void MapMemory();

		ID3D12Resource* GetBuffer() const noexcept;
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;

		BufferData GetData() noexcept;

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
		D3D12MA::Allocation* m_Allocation{ nullptr };

		D3D::D3D12Descriptor m_Descriptor;

		BufferData m_BufferData{};
	};
}
