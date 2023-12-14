#pragma once
#include "Window.hpp"
#include "../LowerDeck Engine/Render/Camera.hpp"
#include "../LowerDeck Engine/Input/CameraInput.hpp"
#include "../LowerDeck Engine/Render/Renderer.hpp"
#include "../LowerDeck Engine/Scene/Scene.hpp"
#include "Editor.hpp"
#include "Timer.hpp"
//#include "../D3D/D3D12Context.hpp"
#include <memory>

// TODO: Put application timer and window here
class Application : public Window
{
public:
	explicit Application(HINSTANCE hInstance);
	//Application(const Application&) = delete;
	//Application(const Application&&) = delete;
	//Application operator=(const Application&) = delete;
	~Application();

	void Initialize();
	/// Render loop
	void Run();

	LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) final;
	void OnResize() final;

	void Release();

private:
	std::shared_ptr<Timer>				m_AppTimer;
	std::unique_ptr<Camera>				m_SceneCamera;
	std::shared_ptr<CameraInput>		m_SceneCameraInputs;
	std::shared_ptr<Editor>				m_Editor;
	std::shared_ptr<D3D::D3D12Context>	m_D3D12Context;
	std::unique_ptr<Renderer>			m_Renderer;
	Scene* m_Scene{ nullptr };


};
