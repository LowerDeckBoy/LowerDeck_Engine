#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx12.h>
#include <vector>
#include <string>

class Window;
class Timer;

namespace D3D { class D3D12Context; }

class Editor
{
	Editor(D3D::D3D12Context* pD3DContext, Timer* pTimer);
	~Editor();

	void OnBeginFrame();

	void OnEndFrame();

	void DrawMenuBar();
	void DrawScene();
	void DrawHierarchy();
	void DrawProperty();
	void DrawContent();
	void DrawLog();

private:
	Window* m_Window{ nullptr };
	D3D::D3D12Context* m_D3D{ nullptr };
	Timer* m_Timer{ nullptr };


	ImFont* m_EditorFont{ nullptr };
	ImGuiViewport* m_EditorViewport{ nullptr };

	std::vector<std::string> m_EditorLogs;

	//ECS::Entity m_SelectedEntity;

	void Initialize(D3D::D3D12Context* pD3DContext, Timer* pTimer);

private:

};
