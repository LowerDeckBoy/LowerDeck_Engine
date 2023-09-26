#include "BufferUtility.hpp"
#include "../../Utility/Utility.hpp"

namespace gfx
{
	Buffer::Buffer(BufferData Data, BufferType Usage, bool bSRV)
	{
		Create(Data, Usage, bSRV);
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::Create(BufferData Data, BufferType Usage, bool bSRV)
	{
		m_BufferData = Data;
		const auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(Data.Size) };

		auto allocFlags{ (D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED |
							D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY |
							D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_BEST_FIT) };
		D3D::HeapAllocation(m_Buffer.Get(), m_Allocation, heapDesc, allocFlags, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

		D3D12MA::Allocation* uploadHeapAllocation{ nullptr };
		ID3D12Resource* uploadHeap{ nullptr };

		const auto& uploadDesc{ D3D::Utility::UploadHeap(true) };
		ThrowIfFailed(D3D::g_Allocator->CreateResource(&uploadDesc, &heapDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &uploadHeapAllocation, IID_PPV_ARGS(&uploadHeap)));

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData		= Data.pData;
		subresource.RowPitch	= Data.Size;
		subresource.SlicePitch	= Data.Size;

		::UpdateSubresources(D3D::g_CommandList.Get(), m_Buffer.Get(), uploadHeap, 0, 0, 1, &subresource);

		// Transit resource to appropiate state.
		switch (Usage)
		{
			case BufferType::eVertex:
			case BufferType::eConstant:
			{
				D3D::TransitResource(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
				break;
			}
			case BufferType::eIndex:
			{
				D3D::TransitResource(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
				break;
			}
			default: 
			{
				uploadHeapAllocation->Release();
				throw std::logic_error("Invalid buffer type!"); 
			}
		}

		D3D::ExecuteCommandLists(true);

		uploadHeapAllocation->Release();

		if (bSRV)
		{
			CreateSRV(m_Buffer.Get(), Data, m_Descriptor);
			//D3D::D3D12Context::GetMainHeap()->Allocate(m_Descriptor);
			//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			//srvDesc.Buffer.FirstElement = 0;
			//srvDesc.Buffer.NumElements = Data.ElementsCount;
			//srvDesc.Buffer.StructureByteStride = Data.Stride;
			//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			//D3D::g_Device.Get()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_Descriptor.GetCPU());
		}
	}

	void Buffer::Release()
	{
		// Note: Release D3D12MA::Allocation BEFORE releasing actual buffer!
		m_Allocation->Release();
		m_Allocation = nullptr;
		SAFE_RELEASE(m_Buffer);
	}

	void CreateSRV(ID3D12Resource* pResource, BufferData Data, D3D::D3D12Descriptor& Descriptor)
	{
		D3D::D3D12Context::GetMainHeap()->Allocate(Descriptor);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = Data.ElementsCount;
		srvDesc.Buffer.StructureByteStride = Data.Stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		D3D::g_Device.Get()->CreateShaderResourceView(pResource, &srvDesc, Descriptor.GetCPU());
	}
}
