#include "Buffer.hpp"

namespace gfx
{
	VertexBuffer::VertexBuffer(BufferData Data, bool bSRV)
	{
		Create(Data, bSRV);
	}

	void VertexBuffer::Create(BufferData Data, bool bSRV)
	{
		Buffer::Create(Data, BufferType::eVertex, bSRV);
		SetView();
	}

	void VertexBuffer::Release()
	{
		View = {};
		Buffer::Release();
	}

	void VertexBuffer::SetView()
	{
		View.BufferLocation = Buffer::GetGPUAddress();
		View.SizeInBytes	= static_cast<uint32_t>(Buffer::GetData().Size);
		View.StrideInBytes	= static_cast<uint32_t>(Buffer::GetData().Size) / Buffer::GetData().ElementsCount;
	}

	IndexBuffer::IndexBuffer(BufferData Data, bool bSRV)
	{
		Create(Data, bSRV);
	}

	void IndexBuffer::Create(BufferData Data, bool bSRV)
	{
		Buffer::Create(Data, BufferType::eIndex, bSRV);
		SetView();
	}

	void IndexBuffer::Release()
	{
		View = {};
		Buffer::Release();
	}

	void IndexBuffer::SetView()
	{
		View.BufferLocation = Buffer::GetGPUAddress();
		View.Format			= DXGI_FORMAT_R32_UINT;
		View.SizeInBytes	= static_cast<uint32_t>(Buffer::GetData().Size);
		Count				= Buffer::GetData().ElementsCount;
	}
}
