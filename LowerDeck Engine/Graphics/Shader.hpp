#pragma once

namespace gfx
{
	class Shader
	{
	public:
		/// <summary>Initialize with Shader Blob.</summary>
		/// <param name="pDxc"> Pointer to <c>Dxc Shader Blob</c>. </param>
		Shader(IDxcBlob* pDxc) : m_DxcCode(pDxc) {}
		/// <summary> Auto release. </summary>
		~Shader() { if (m_DxcCode) { m_DxcCode.Reset(); m_DxcCode = nullptr; } }

		/// <summary> Gets <c>void pointer</c> to shader data straight from <c>Dxc blob</c>.</summary>
		/// <returns> Shader bytecode data. </returns>
		inline void* Code() { return m_DxcCode->GetBufferPointer(); }
		/// <summary> Get size of a shader straight from <c>Dxc blob</c>. </summary>
		/// <returns> Size of a shader bytecode. </returns>
		inline size_t Size() { return m_DxcCode->GetBufferSize(); }

		/// <summary> Wrapper for getting shader data for pipeline building. </summary>
		/// <returns> Shader code and it's size. </returns>
		inline D3D12_SHADER_BYTECODE Bytecode() { return D3D12_SHADER_BYTECODE(Code(), Size()); }
	private:
		Microsoft::WRL::ComPtr<IDxcBlob> m_DxcCode;

	};
}
