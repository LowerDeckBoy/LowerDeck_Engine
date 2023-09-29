#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx12.h>

class Camera;
class Timer;

/// <summary>
/// <b>ImGui</b> based.
/// </summary>
class Editor
{
public:
	Editor() = default;
	~Editor();

	void Initialize(Camera* pCamera, Timer* pEngineTimer);

	/// <summary>
	/// Goes after reseting commend lists.
	/// </summary>
	void OnFrameBegin();

	/// <summary>
	/// Goes before making Barrier Resource Transition for Present state
	/// </summary>
	void OnFrameEnd();

	void Release();

	//std::vector<>

	float GetWidth() { return m_ViewportWidth; }
	float GetHeight() { return m_ViewportHeigth; }

private:
	/// <summary>
	/// Viewport area
	/// </summary>
	ImGuiViewport*  m_MainViewport{ nullptr };
	/// <summary>
	/// CascadiaCode Bold by default.
	/// </summary>
	ImFont*			m_MainFont{ nullptr };

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
