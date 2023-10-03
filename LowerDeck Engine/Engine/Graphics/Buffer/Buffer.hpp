#pragma once
#include "../../D3D/D3D12Context.hpp"
#include "BufferUtility.hpp"
#include "Vertex.hpp"

namespace gfx
{
	// Buffer are separated into different class
	// as they use different View types.
	
	/// <summary>
	/// Inherits from <see cref="Buffer"></see> class.
	/// </summary>
	class VertexBuffer : public Buffer
	{
	public:
		VertexBuffer() = default;
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="bSRV"> Indicates whether ShaderResourceView should be allocated. </param>
		VertexBuffer(BufferData Data, bool bSRV = false);
		/// <summary>
		/// 
		/// </summary>
		//~VertexBuffer() { Release(); }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="bSRV">Indicates whether ShaderResourceView should be allocated.</param>
		void Create(BufferData Data, bool bSRV = false);

		/// <summary>
		/// 
		/// </summary>
		//void Release() override;

		/// <summary>
		/// 
		/// </summary>
		D3D12_VERTEX_BUFFER_VIEW View{};
	private:
		void SetView();
	};

	/// <summary>
	/// Inherits from <c>Buffer</c> class.
	/// </summary>
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer() = default;
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="bSRV">Indicates whether ShaderResourceView should be allocated.</param>
		IndexBuffer(BufferData Data, bool bSRV = false);
		//~IndexBuffer() { Release(); }
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Data"></param>
		/// <param name="bSRV">Indicates whether ShaderResourceView should be allocated.</param>
		void Create(BufferData Data, bool bSRV = false);

		/// <summary>
		/// 
		/// </summary>
		//void Release() override;

		// TODO: Add indicating whether indices are of 16 or 32 bits size
		// Might result greatly in final total sizes

		/// <summary>
		/// 
		/// </summary>
		D3D12_INDEX_BUFFER_VIEW View{};
		/// <summary>
		/// Number of total indices hold by this buffer.
		/// </summary>
		uint32_t Count{ 0 };
	private:
		void SetView();
	};

	/// <summary>
	/// Buffer that is meant to be accessed (read-only) by shader.
	/// </summary>
	class StructureBuffer : public Buffer
	{
	public:
		StructureBuffer() = default;
		StructureBuffer(BufferData Data, bool bSRV = true);

		void Create(BufferData Data, bool bSRV = true);

		D3D::D3D12Descriptor GetDescriptor() { return m_Descriptor; }
	};

}
