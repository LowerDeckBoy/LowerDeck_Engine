#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx12.h>

/// <summary>
/// <b>ImGui</b> based.
/// </summary>
class Editor
{
public:
	Editor() = default;
	~Editor();

	void Initialize();

	/// <summary>
	/// Goes after reseting commend lists.
	/// </summary>
	void OnFrameBegin();

	/// <summary>
	/// Goes before making Barrier Resource Transition for Present state
	/// </summary>
	void OnFrameEnd();

	void Release();

private:
	/// <summary>
	/// Viewport area
	/// </summary>
	ImGuiViewport*  m_MainViewport{ nullptr };
	/// <summary>
	/// CascadiaCode Bold by default.
	/// </summary>
	ImFont*			m_MainFont{ nullptr };

	/// <summary>Image output Viewport width</summary>
	float m_ViewportWidth{};
	/// <summary>Image output Viewport height</summary>
	float m_ViewportHeigth{};

};
