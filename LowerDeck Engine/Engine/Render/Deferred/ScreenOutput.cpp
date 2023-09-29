#include "ScreenOutput.hpp"
//#include "../../Graphics/Buffer/Vertex.hpp"


ScreenOutput::~ScreenOutput()
{
}

void ScreenOutput::Create()
{
	std::array<gfx::ScreenOutputVertex, 4> vertices =
	{
		gfx::ScreenOutputVertex{ { -1.0f, +1.0f, 0.0f }, { 0.0f, 0.0f } },
		gfx::ScreenOutputVertex{ { +1.0f, +1.0f, 0.0f }, { 1.0f, 0.0f } },
		gfx::ScreenOutputVertex{ { +1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
		gfx::ScreenOutputVertex{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }
	};

	std::array<uint32_t, 6> indices =
	{
		0, 1, 2, 2, 3, 0
	};

	m_VertexBuffer = new gfx::VertexBuffer({ vertices.data(), vertices.size(), sizeof(vertices.at(0)) * vertices.size(), sizeof(vertices.at(0)) });
	m_IndexBuffer = new gfx::IndexBuffer({ indices.data(), indices.size(), sizeof(indices.at(0)) * indices.size(), sizeof(indices.at(0)) });

}

void ScreenOutput::Draw()
{
	D3D::g_CommandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D::g_CommandList.Get()->IASetVertexBuffers(0, 1, &m_VertexBuffer->View);
	D3D::g_CommandList.Get()->IASetIndexBuffer(&m_IndexBuffer->View);
	D3D::g_CommandList.Get()->DrawIndexedInstanced(m_IndexBuffer->Count, 1, 0, 0, 0);
}

void ScreenOutput::Release()
{
	if (m_VertexBuffer)
	{
		delete m_VertexBuffer;
		//m_VertexBuffer->Release();
		//m_VertexBuffer = nullptr;
	}

	if (m_IndexBuffer)
	{
		delete m_IndexBuffer;
		//m_IndexBuffer->Release();
		//m_IndexBuffer = nullptr;
	}
}
