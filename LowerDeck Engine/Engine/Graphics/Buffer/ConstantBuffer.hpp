#pragma once
#include "BufferUtility.hpp"
#include "ConstantTypes.hpp"

namespace gfx
{
	template<typename T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		ConstantBuffer(T* pData)
		{
			Create(pData);
		}
		~ConstantBuffer()
		{
			Release();
		}

		void Create(T* pData);

		void Update(const T& Updated);

		ID3D12Resource* GetBuffer() { return m_Buffers.at(FRAME_INDEX).Get(); }

		void Release();
		
		std::array<uint8_t*, D3D::FRAME_COUNT> pDataBegin{};

	private:
		std::array<ComPtr<ID3D12Resource>, D3D::FRAME_COUNT> m_Buffers;
		std::array<T*, D3D::FRAME_COUNT> m_Data{};

	};

	template<typename T>
	inline void ConstantBuffer<T>::Create(T* pData)
	{
		// Align data to 256 bytes
		const size_t dataSize{ ((sizeof(T) + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) };

		auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(dataSize) };

		for (uint32_t i = 0; i < D3D::FRAME_COUNT; i++)
		{
			m_Data.at(i) = pData;

			const auto flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
			D3D12MA::Allocation* allocation{ nullptr };
			D3D::HeapAllocation(&m_Buffers.at(i), &allocation, bufferDesc, flags, D3D12_HEAP_TYPE_UPLOAD);

			D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
			bufferView.BufferLocation = m_Buffers.at(i).Get()->GetGPUVirtualAddress();
			bufferView.SizeInBytes = static_cast<uint32_t>(dataSize);

			// Persistent mapping
			const CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(m_Buffers.at(i).Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin.at(i))));
			std::memcpy(pDataBegin.at(i), &pData, dataSize);

			allocation->Release();
			allocation = nullptr;
		}
	}

	template<typename T>
	inline void ConstantBuffer<T>::Update(const T& Updated)
	{
		*m_Data.at(FRAME_INDEX) = Updated;
		std::memcpy(pDataBegin.at(FRAME_INDEX), &Updated, sizeof(T));
	}

	template<typename T>
	inline void ConstantBuffer<T>::Release()
	{
		for (uint32_t i = 0; i < D3D::FRAME_COUNT; i++)
		{
			SAFE_RELEASE(m_Buffers.at(i));
			m_Data.at(i) = nullptr;
		}
	}
}
