#pragma once

#include "Win32.h"
#include <string>

class Window
{
protected:
	Window();
	virtual ~Window();

	static LRESULT CALLBACK WndProcRouter(const HWND hWnd, const UINT uMsg,
		const WPARAM wParam, const LPARAM lParam);

	virtual LRESULT CALLBACK WndProc(const HWND hWnd, const UINT uMsg,
		const WPARAM wParam, const LPARAM lParam);

	HWND Create(const LPCWSTR &title, const WNDCLASSEX &wndClass, 
		const int sizeX, const int sizeY, const int posX, const int posY);
	HWND Create(const LPCWSTR &title, const WNDCLASSEX &wndClass,
		const int sizeX, const int sizeY)
	{
		// Windows API defines: CW_USEDEFAULT as 0x80000000
		return Create(title, wndClass, sizeX, sizeY, static_cast<int>(0x80000000), static_cast<int>(0x80000000));
	}
};

