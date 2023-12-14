#pragma once
#include "Dependencies/ImGui/imgui.h"
#include "Dependencies/ImGui/imgui_impl_win32.h"
#include "Dependencies/ImGui/imgui_impl_dx12.h"
#include "../LowerDeck Engine/Core/Logger.hpp"
#include "../LowerDeck Engine/ECS/Entity.hpp"
#include "../LowerDeck Engine/Scene/Scene.hpp"

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

	void Initialize(Renderer* pRenderer, Scene* pScene, Camera* pCamera, Timer* pEngineTimer);

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
	void DrawPropertyPanel();

	void Release();

	//std::vector<>

private:
	void DrawLogs();

	void DrawNode(ecs::Entity& Entity);

	void DrawComponentsData(ecs::Entity& Entity);

	template<typename T, typename UI>
	void DrawProperties(ecs::Entity& Entity, UI ui);

	void DrawFloat3(std::string Label, DirectX::XMFLOAT3& Float3, float ResetValue = 0.0f);
	void DrawFloat4(std::string Label, DirectX::XMFLOAT4& Float4, float ResetValue = 0.0f);

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
	Scene* m_Scene{ nullptr };
	ecs::Entity m_SelectedEntity;

	/// <summary>
	/// Reference to Scene Camera object.
	/// </summary>
	Camera* m_Camera;
	/// <summary>
	/// Reference to Engine Timer object.
	/// </summary>
	Timer* m_Timer;

};
