#include "Window.h"
#include <memory>

LRESULT CALLBACK Window::WndProc(const HWND hWnd, const UINT uMsg, 
		const WPARAM wParam, const LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(EXIT_SUCCESS);
		return NULL;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND Window::Create(const LPCWSTR &title, const WNDCLASSEX &wndClass, 
	const int sizeX, const int sizeY, const int posX, const int posY)
{
	RegisterClassEx(&wndClass);

	RECT wr = { NULL, NULL, sizeX, sizeY };
	AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW, FALSE, NULL);

	HWND windowHandle = CreateWindowEx(
		NULL,
		wndClass.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		posX, posY,
		(wr.right - wr.left), (wr.bottom - wr.top),
		nullptr,
		nullptr,
		wndClass.hInstance,
		this);

	return windowHandle;
}

Window::Window()
{
}


Window::~Window()
{
}

LRESULT CALLBACK Window::WndProcRouter(const HWND hWnd, const UINT uMsg,
	const WPARAM wParam, const LPARAM lParam)
{
	Window* wnd;

	switch (uMsg)
	{
	case WM_NCCREATE:
		
		// Register window
		wnd = reinterpret_cast<Window*>(
			(reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 
			reinterpret_cast<LONG_PTR>(wnd));
		
		break;

	case WM_NCDESTROY:

		// Deregister window
		wnd = reinterpret_cast<Window*>(
			(reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 
			reinterpret_cast<LONG_PTR>(nullptr));

		break;

	default:

		// Call window specific procedure, if available
		wnd = reinterpret_cast<Window*>(
			GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (wnd)
		{
			return wnd->WndProc(hWnd, uMsg, wParam, lParam);
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}