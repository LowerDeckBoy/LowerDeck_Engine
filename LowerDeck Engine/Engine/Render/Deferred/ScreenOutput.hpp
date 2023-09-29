#pragma once
#include "../../Graphics/Buffer/Buffer.hpp"
// TODO: add some namespace

/// <summary>
/// Used to create screen quad for displaying deferred rendering output.
/// </summary>
class ScreenOutput
{
public:
	ScreenOutput() = default;
	ScreenOutput(const ScreenOutput&) = delete;
	ScreenOutput(const ScreenOutput&&) = delete;
	ScreenOutput operator=(const ScreenOutput&&) = delete;
	~ScreenOutput();

	void Create();

	void Draw();

	void Release();

private:
	gfx::VertexBuffer* m_VertexBuffer{ nullptr };
	gfx::IndexBuffer* m_IndexBuffer{ nullptr };

};
