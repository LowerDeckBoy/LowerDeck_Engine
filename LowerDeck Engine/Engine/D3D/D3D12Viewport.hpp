#pragma once
#include "D3D12Utility.hpp"

namespace D3D
{
	/// <summary>
	/// D3D12_VIEWPORT and D3D12_RECT wrapper.<br/>
	/// D3D12Viewport is meant to be used as a class object
	/// for later reusability.<br/>
	/// Example usage:<br/>
	/// - main renderer viewport and scissor,<br/>
	/// - shadow mapping viewport per light source,<br/>
	/// </summary>
	class D3D12Viewport
	{
	public:
		D3D12Viewport() = default;
		D3D12Viewport(uint32_t Width, uint32_t Height);
		~D3D12Viewport() = default;

		/// Set same dimensions for both Viewport and Scissor
		void Set(uint32_t Width, uint32_t Height);
		/// Set dimensions for Viewport only
		void SetViewport(uint32_t Width, uint32_t Height);
		/// Set dimensions for Scissor only
		void SetScissor(uint32_t Width, uint32_t Height);

		/// <summary> Viewport getter </summary>
		/// <returns> Private member of type: D3D12_VIEWPORT</returns>
		[[nodiscard]] inline const D3D12_VIEWPORT& Viewport() const { return m_Viewport; }
		/// <summary> Scissor getter </summary>
		/// <returns> Private member of type: D3D12_RECT </returns>
		[[nodiscard]] inline const D3D12_RECT& Scissor() const { return m_Scissor; }

	private:
		D3D12_VIEWPORT	m_Viewport{};
		D3D12_RECT		m_Scissor{};
	};
}
