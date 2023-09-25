#include "BufferUtility.hpp"
#include "../../Utility/Utility.hpp"

namespace gfx
{
	Buffer::~Buffer()
	{
	}

	void Buffer::Create(BufferData Data, BufferType TypeOf, bool bSRV)
	{
		m_BufferData = Data;
		const auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(Data.Size) };

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		allocDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
		allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		ThrowIfFailed(D3D::g_Allocator->CreateResource(&allocDesc, &heapDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &m_Allocation, IID_PPV_ARGS(m_Buffer.ReleaseAndGetAddressOf())));

		D3D12MA::ALLOCATION_DESC uploadDesc{};
		uploadDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		uploadDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_BEST_FIT;
		uploadDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

		D3D12MA::Allocation* uploadHeapAllocation{ nullptr };
		ID3D12Resource* uploadHeap{ nullptr };
		ThrowIfFailed(D3D::g_Allocator->CreateResource(&uploadDesc, &heapDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &uploadHeapAllocation, IID_PPV_ARGS(&uploadHeap)));

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = Data.pData;
		subresource.RowPitch = Data.Size;
		subresource.SlicePitch = Data.Size;

		::UpdateSubresources(D3D::g_CommandList.Get(), m_Buffer.Get(), uploadHeap, 0, 0, 1, &subresource);

		if ((TypeOf & BufferType::eVertex) || (TypeOf & BufferType::eConstant))
		{
			const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
			D3D::g_CommandList.Get()->ResourceBarrier(1, &barrier);
		}
		else if (TypeOf & BufferType::eIndex)
		{
			const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER) };
			D3D::g_CommandList.Get()->ResourceBarrier(1, &barrier);
		}
		else
		{
			uploadHeapAllocation->Release();
			throw std::logic_error("Invalid buffer type!");
		}

		D3D::ExecuteCommandLists(true);

		uploadHeapAllocation->Release();

		if (bSRV)
		{
			D3D::D3D12Context::GetMainHeap()->Allocate(m_Descriptor);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = Data.ElementsCount;
			srvDesc.Buffer.StructureByteStride = Data.Stride;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			D3D::g_Device.Get()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_Descriptor.GetCPU());
		}
	}

	void Buffer::MapMemory()
	{
	}

	ID3D12Resource* Buffer::GetBuffer() const noexcept
	{
		return nullptr;
	}

	D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUAddress() const
	{
		return D3D12_GPU_VIRTUAL_ADDRESS();
	}

	BufferData Buffer::GetData() noexcept
	{
		return BufferData();
	}
}
