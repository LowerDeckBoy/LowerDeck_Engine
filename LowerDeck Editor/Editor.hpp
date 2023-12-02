#pragma once
#include "Dependencies/ImGui/imgui.h"
#include "Dependencies/ImGui/imgui_impl_win32.h"
#include "Dependencies/ImGui/imgui_impl_dx12.h"
#include "../LowerDeck Engine/Core/Logger.hpp"

class Camera;
class Timer;
class Renderer;

/// <summary>
/// <b>ImGui</b> based.
/// </summary>
class Editor
{
public:
	Editor() = default;
	~Editor();

	void Initialize(Renderer* pRenderer, Camera* pCamera, Timer* pEngineTimer);

	/// <summary>
	/// Goes after reseting commend lists.
	/// </summary>
	void OnFrameBegin();

	/// <summary>
	/// Goes before making Barrier Resource Transition for Present state
	/// </summary>
	void OnFrameEnd();

	void DrawMenuBar();
	void DrawScene();
	void DrawHierarchy();
	void DrawProperties();


	void Release();

	//std::vector<>

	float GetWidth() { return m_ViewportWidth; }
	float GetHeight() { return m_ViewportHeigth; }

private:
	void DrawLogs();

private:
	/// <summary>
	/// Viewport area
	/// </summary>
	ImGuiViewport* m_MainViewport{ nullptr };
	/// <summary>
	/// CascadiaCode Bold by default.
	/// </summary>
	ImFont* m_MainFont{ nullptr };

	Renderer* m_Renderer{ nullptr };

	/// <summary>
	/// Reference to Scene Camera object.
	/// </summary>
	Camera* m_Camera;
	/// <summary>
	/// Reference to Engine Timer object.
	/// </summary>
	Timer* m_Timer;

	/// <summary>Image output Viewport width</summary>
	float m_ViewportWidth{};
	/// <summary>Image output Viewport height</summary>
	float m_ViewportHeigth{};

};
