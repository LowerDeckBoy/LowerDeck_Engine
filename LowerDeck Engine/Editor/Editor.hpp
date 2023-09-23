#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx12.h>


class Editor
{
public:
	Editor() = default;
	~Editor();

	void Initialize();

	void OnFrameBegin();

	/// <summary>
	/// Goes before making Barrier Resource Transition for Present state
	/// </summary>
	void OnFrameEnd();

	void Release();

private:

	ImGuiViewport*  m_MainViewport{ nullptr };
	ImFont*			m_MainFont{ nullptr };

};
