#pragma once
#include "../Graphics/Buffer/Buffer.hpp"

/// <summary>
/// Used to create screen quad for displaying deferred rendering output.
/// </summary>
class ScreenOutput
{
public:
	/// <summary> Initializes object. </summary>
	ScreenOutput();
	/// <summary> Coping isn't allowed. </summary>
	/// <param name=""> RHS </param>
	ScreenOutput(const ScreenOutput&) = delete;
	/// <summary> Coping isn't allowed. </summary>
	/// <param name=""> RHS </param>
	ScreenOutput(const ScreenOutput&&) = delete;
	/// <summary> Coping isn't allowed. </summary>
	/// <param name=""></param>
	/// <returns> Deleted. </returns>
	ScreenOutput operator=(const ScreenOutput&&) = delete;
	/// <summary> Releases object. </summary>
	~ScreenOutput();

	/// <summary>
	/// Initializes buffers.
	/// </summary>
	void Create();

	/// <summary>
	/// Draws quad for deferred output.
	/// </summary>
	void Draw();

	/// <summary>
	/// Releases buffers.
	/// </summary>
	void Release();

private:
	/// <summary>
	/// Underlying Vertex Buffer.
	/// </summary>
	gfx::VertexBuffer* m_VertexBuffer{ nullptr };
	/// <summary>
	/// Underlying Index Buffer.
	/// </summary>
	gfx::IndexBuffer* m_IndexBuffer{ nullptr };

};
