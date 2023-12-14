#include "Window.hpp"
#include <cstdlib>
#include <cassert>

#pragma comment(lib, "dwmapi")

HINSTANCE Window::m_hInstance = nullptr;
HWND Window::m_hWnd = nullptr;
bool Window::bCursorVisible = true;

namespace { Window* window = 0; }
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return window->WindowProc(hWnd, msg, wParam, lParam);
}

Window::Window(HINSTANCE hInstance)
{
	::window = this;
	m_hInstance = hInstance;

	m_Resolution.Width = 1280;
	m_Resolution.Height = 800;
	m_Resolution.AspectRatio = static_cast<float>(m_Resolution.Width) / static_cast<float>(m_Resolution.Height);
}

Window::~Window()
{
	Release();
}

bool Window::Initialize()
{
	if (bInitialized)
	{
		return false;
	}

	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_hInstance;
	wcex.lpszClassName = m_WindowClass;
	wcex.lpfnWndProc = MsgProc;
	wcex.hbrBackground = ::CreateSolidBrush(RGB(32, 32, 32));

	if (!::RegisterClassEx(&wcex))
	{
		std::exit(EXIT_FAILURE);
	}

	m_WindowRect = { 0, 0, static_cast<LONG>(m_Resolution.Width),static_cast<LONG>(m_Resolution.Height) };
	::AdjustWindowRect(&m_WindowRect, WS_OVERLAPPEDWINDOW, false);

	const int32_t width = static_cast<int32_t>(m_WindowRect.right - m_WindowRect.left);
	const int32_t height = static_cast<int32_t>(m_WindowRect.bottom - m_WindowRect.top);

	m_hWnd = ::CreateWindow(
		m_WindowClass, m_WindowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, 0, 0,
		m_hInstance, 0);

	assert(m_hWnd && "Failed to create Window Handle!\n");

	// Centering window position upon first appearing
	const int32_t xPos = (::GetSystemMetrics(SM_CXSCREEN) - m_WindowRect.right) / 2;
	const int32_t yPos = (::GetSystemMetrics(SM_CYSCREEN) - m_WindowRect.bottom) / 2;

	::SetWindowPos(m_hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	try
	{
		//const BOOL bDarkMode{ TRUE };
		//::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

		
	}
	catch (...)
	{
		// warning log
		// couldn't set dark mode
	}

	bInitialized = true;
	return true;
}

void Window::Show()
{
	//COLORREF captionColor{ RGB(32, 32, 32) };
	//DwmSetWindowAttribute(m_hWnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));

	::ShowWindow(m_hWnd, SW_SHOW);
	::SetForegroundWindow(m_hWnd);
	::SetFocus(m_hWnd);
	::UpdateWindow(m_hWnd);
}

void Window::Release()
{
	::UnregisterClass(m_WindowClass, m_hInstance);

	if (m_hWnd)		 m_hWnd = nullptr;
	if (m_hInstance) m_hInstance = nullptr;
}

void Window::Run()
{
	Show();
	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

	}
}

void Window::SetResolution(uint32_t Width, uint32_t Height)
{
	m_Resolution.Width = Width;
	m_Resolution.Height = Height;
	m_Resolution.AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
}

void Window::ShowCursor() noexcept
{
	while (::ShowCursor(true) < 0)
		bCursorVisible = true;
}

void Window::HideCursor() noexcept
{
	while (::ShowCursor(false) >= 0)
		bCursorVisible = false;
}
