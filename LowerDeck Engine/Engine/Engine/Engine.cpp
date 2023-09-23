#include "Engine.hpp"

Engine::Engine(HINSTANCE hInstance) 
	: Window(hInstance)
{
}

Engine::~Engine()
{
}

void Engine::Initialize()
{
	m_EngineTimer = std::make_unique<Timer>();
	Window::Initialize();

	m_SceneCamera = std::make_unique<Camera>();
	m_SceneCameraInputs = std::make_unique<CameraInput>();
	
	//m_Editor = std::make_unique<Editor>();

	m_Renderer = std::make_unique<Renderer>(m_SceneCamera);
	//m_Editor->Initialize();
}

void Engine::Run()
{
	// If app is ready to render show actual window
	// otherwise window would be blank until 
	// resources are ready to render
	Window::Show();

	// Stop timer on Resize event 
	// so Backbuffer resizing can't be done
	// without ongoing frame rendering
	// m_EngineTimer->Start();

	m_SceneCameraInputs->Initialize();
	// Ensure Timer clean start
	m_EngineTimer->Reset();
	m_SceneCamera->ResetCamera();

	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		// Process messages
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			m_EngineTimer->Tick();
			m_EngineTimer->GetFrameStats();

			if (!bAppPaused)
			{
				m_SceneCameraInputs->ProcessInputs(m_SceneCamera, m_EngineTimer->DeltaTime());
				
				m_Renderer->Update();
				m_Renderer->Render();

				m_SceneCamera->Update();
			}
			else
				::Sleep(100);
		}
	}
}

void Engine::OnResize()
{
}

void Engine::Release()
{
	m_Renderer->Release();
}

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT Engine::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	//if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
	//	return true;

	switch (Msg)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			m_EngineTimer->Stop();
		}
		else
		{
			bAppPaused = false;
			m_EngineTimer->Start();
		}
		return 0;
	}
	case WM_SIZE:
	{
		Window::SetResolution(static_cast<uint32_t>(LOWORD(lParam)), static_cast<uint32_t>(HIWORD(lParam)));
		//m_Display.Width  = static_cast<float>(LOWORD(lParam));
		//m_Display.Height = static_cast<float>(HIWORD(lParam));
		//m_Display.AspectRatio = (m_Display.Width / m_Display.Height);

		if (!m_Renderer)
		{
			return 0;
		}
		if (wParam == SIZE_MINIMIZED)
		{
			bAppPaused = true;
			bMinimized = true;
			bMaximized = false;

		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			bAppPaused = false;
			bMinimized = false;
			bMaximized = true;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (bMinimized)
			{
				bAppPaused = false;
				bMinimized = false;
			}
			else if (bMaximized)
			{
				bAppPaused = false;
				bMaximized = false;
			}
			OnResize();
		}

		return 0;
	}
	case WM_ENTERSIZEMOVE:
	{
		bAppPaused = true;
		bIsResizing = true;
		m_EngineTimer->Stop();

		return 0;
	}
	case WM_EXITSIZEMOVE:
	{
		bAppPaused = false;
		bIsResizing = false;

		//OnResize();
		m_EngineTimer->Start();

		return 0;
	}
	case WM_CLOSE:
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		return 0;
	}
	}

	return ::DefWindowProcW(hWnd, Msg, wParam, lParam);
}
