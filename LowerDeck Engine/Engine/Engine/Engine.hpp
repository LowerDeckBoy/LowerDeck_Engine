#pragma once
#include "../Window/Window.hpp"
#include "../Render/Camera.hpp"
#include "../Input/CameraInput.hpp"
#include "../Editor/Editor.hpp"
#include "../Utility/Timer.hpp"
#include "../Engine/Engine/Renderer.hpp"
//#include "../D3D/D3D12Context.hpp"
#include <memory>

// TODO:
// Restructure logic

/// <summary>
/// Application entry point.<br></br>
/// Responsbile for render-loop.<br></br>
/// <c>Engine</c> inherits from Window class in order to
/// override WinAPI Window Procedures and enable proper Resize logic.
/// </summary>
class Engine : public Window
{
public:
	explicit Engine(HINSTANCE hInstance);
	Engine(const Engine&) = delete;
	Engine(const Engine&&) = delete;
	Engine operator=(const Engine&) = delete;
	~Engine();

	void Initialize();
	/// Render loop
	void Run();

	LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) final;
	void OnResize() final;

	void Release();

private:
	std::shared_ptr<Timer>				m_EngineTimer;
	std::unique_ptr<Camera>				m_SceneCamera;
	std::shared_ptr<CameraInput>		m_SceneCameraInputs;
	std::shared_ptr<Editor>				m_Editor;
	std::shared_ptr<D3D::D3D12Context>	m_D3D12Context;
	std::unique_ptr<Renderer>			m_Renderer;


};

