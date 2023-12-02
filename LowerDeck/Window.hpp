#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstdint>


class Window
{
public:
	explicit Window(HINSTANCE hInstance);
	Window(const Window&) = delete;
	Window(const Window&&) = delete;
	Window operator=(const Window&) = delete;
	virtual ~Window();

	virtual LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnResize() = 0;

	bool Initialize();
	void Show();
	void Release();

	void Run();

	void SetResolution(uint32_t Width, uint32_t Height);

	static void ShowCursor() noexcept;
	static void HideCursor() noexcept;

	static const HINSTANCE& GetHInstance() { return m_hInstance; };
	static const HWND& GetHwnd() { return m_hWnd; };

	inline static struct DisplayResolution
	{
		uint32_t Width;
		uint32_t Height;
		float	 AspectRatio;
	} m_Resolution;

	inline static bool bShouldQuit{ false };

protected:
	static HINSTANCE m_hInstance;
	static HWND		 m_hWnd;

	RECT m_WindowRect{};

	LPCWSTR m_WindowClass{ L"LowerDeck" };
	LPCWSTR m_WindowName{ L"LowerDeck" };

	bool bInitialized{ false };

protected:
	static bool bCursorVisible;

	// Managing application states via Message Procedures
	bool bAppPaused{ false };
	bool bMinimized{ false };
	bool bMaximized{ false };
	bool bIsResizing{ false };


};
