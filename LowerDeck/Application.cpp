#include "Application.hpp"
#include "Dependencies/ImGui/imgui_impl_win32.h"

Application::Application(HINSTANCE hInstance)
	: Window(hInstance)
{
	m_AppTimer = std::make_unique<Timer>();
}

Application::~Application()
{
}

void Application::Initialize()
{
	Window::Initialize();

	m_D3D12Context = std::make_shared<D3D::D3D12Context>();
	m_D3D12Context->InitializeD3D();

	m_SceneCamera = std::make_unique<Camera>();
	m_SceneCameraInputs = std::make_shared<CameraInput>();
	m_SceneCameraInputs->Initialize();

	m_Scene = new Scene();
	m_Renderer = std::make_unique<Renderer>(m_D3D12Context, m_SceneCamera.get());

	m_Editor = std::make_shared<Editor>();
	m_Editor->Initialize(m_Renderer.get(), m_Scene, m_SceneCamera.get(), m_AppTimer.get());

}

void Application::Run()
{
	Window::Show();
	::MSG msg{};

	// Ensure clean timer before app starts rendering
	m_AppTimer->Reset();
	m_SceneCamera->ResetCamera();

	while (!bShouldQuit)
	{
		if (msg.message == WM_QUIT)
		{
			bShouldQuit = true;
			continue;
		}
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			m_AppTimer->Tick();
			m_AppTimer->GetFrameStats();

			if (!bAppPaused)
			{
				m_SceneCameraInputs->ProcessInputs(m_SceneCamera.get(), m_AppTimer->DeltaTime());

				m_Editor->OnFrameBegin();
				m_Renderer->Update();
				m_Renderer->Render();
				m_Editor->OnFrameEnd();
				m_Renderer->Present();

				m_SceneCamera->Update();
			}
			else
				::Sleep(100);
		}
	}
	
	//m_D3D12Context->Idle();
}

void Application::OnResize()
{
	m_Renderer->OnResize();
	m_SceneCamera->OnAspectRatioChange(Window::m_Resolution.AspectRatio);
}

void Application::Release()
{
	//m_Scene = nullptr;
	//m_Renderer.reset();

#if EDITOR_MODE
	m_Editor.reset();
	m_Editor = nullptr;
#endif

	//delete m_Gfx;
}

LRESULT Application::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return true;

	switch (Msg)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			m_AppTimer->Stop();
		}
		else
		{
			bAppPaused = false;
			m_AppTimer->Start();
		}

		return 0;
	}
	case WM_SIZE:
	{
		Window::m_Resolution.Width = static_cast<uint32_t>(LOWORD(lParam));
		Window::m_Resolution.Height = static_cast<uint32_t>(HIWORD(lParam));
		Window::m_Resolution.AspectRatio = static_cast<float>(Window::m_Resolution.Width) / static_cast<float>(Window::m_Resolution.Height);
		//if (!m_Renderer)
		//{
		//	LOG_CRITICAL("Renderer not found!\n");
		//	return 0;
		//}
		if (wParam == SIZE_MINIMIZED)
		{
			bAppPaused = true;
			bMinimized = true;
			bMaximized = false;
			::ShowWindow(Window::GetHwnd(), SW_MINIMIZE);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			bAppPaused = false;
			bMinimized = false;
			bMaximized = true;
			OnResize();
			::ShowWindow(Window::GetHwnd(), SW_MAXIMIZE);

		}
		else if (wParam == SIZE_RESTORED)
		{
			if (bMinimized)
			{
				bAppPaused = false;
				bMinimized = false;
				::ShowWindow(Window::GetHwnd(), SW_NORMAL);
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
		m_AppTimer->Stop();

		return 0;
	}
	case WM_EXITSIZEMOVE:
	{
		bAppPaused = false;
		bIsResizing = false;

		//OnResize();
		m_AppTimer->Start();

		return 0;
	}
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			::PostQuitMessage(0);
		}
		return 0;
	case WM_CLOSE:
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	}

	return ::DefWindowProc(hWnd, Msg, wParam, lParam);

}
