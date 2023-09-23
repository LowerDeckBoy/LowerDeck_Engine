#include "D3D12Viewport.hpp"

namespace D3D
{
	D3D12Viewport::D3D12Viewport(uint32_t Width, uint32_t Height)
	{
		Set(Width, Height);
	}

	void D3D12Viewport::Set(uint32_t Width, uint32_t Height)
	{
		SetViewport(Width, Height);
		SetScissor(Width, Height);
	}

	void D3D12Viewport::SetViewport(uint32_t Width, uint32_t Height)
	{
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width	= static_cast<float>(Width);
		m_Viewport.Height	= static_cast<float>(Height);

		m_Viewport.MinDepth = D3D12_MIN_DEPTH;
		m_Viewport.MaxDepth = D3D12_MAX_DEPTH;
	}

	void D3D12Viewport::SetScissor(uint32_t Width, uint32_t Height)
	{
		m_Scissor.left	 = 0L;
		m_Scissor.top	 = 0L;
		m_Scissor.right  = static_cast<LONG>(Width);
		m_Scissor.bottom = static_cast<LONG>(Height);
	}
}
