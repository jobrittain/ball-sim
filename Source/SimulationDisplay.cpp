#include "SimulationDisplay.h"
#include <AntTweakBar.h>
#include <Mouse.h>
#include <Keyboard.h>

LRESULT SimulationDisplay::WndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
	if (TwEventWin(hWnd, uMsg, wParam, lParam))
		return NULL;

	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
		DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
		break;
	}

	return Window::WndProc(hWnd, uMsg, wParam, lParam);
}

SimulationDisplay::SimulationDisplay(HINSTANCE instanceHandle) : 
	_instanceHandle(instanceHandle),
	_screenWidth(InitialScreenWidth),
	_screenHeight(InitialScreenHeight)
{
	_graphicsDevice = std::make_shared<GraphicsDevice>();
}


SimulationDisplay::~SimulationDisplay()
{
}


HWND SimulationDisplay::Initialize(SimulationSystem * const simulationSystem)
{
	// REGISTER SYSTEM

	_simulationSystem = simulationSystem;

	// CREATE THE WINDOW

	WNDCLASSEX wnd = {};

	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.style = CS_VREDRAW | CS_HREDRAW;
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hInstance = _instanceHandle;
	wnd.lpszClassName = L"SimulationWindow";
	wnd.lpfnWndProc = static_cast<WNDPROC>(WndProcRouter);

	_windowHandle = Create(L"Gravity Wells", wnd,
		static_cast<int>(_screenWidth), static_cast<int>(_screenHeight));

	// INITIALLIZE GRAPHICS DEVICE

	_graphicsDevice->Initialize(_windowHandle, _screenWidth, _screenHeight);

	// INITIALIZE ANTTWEAKBAR

	TwInit(TW_DIRECT3D11, _graphicsDevice->Device());
	TwWindowSize(static_cast<int>(_screenWidth), static_cast<int>(_screenWidth));

	return _windowHandle;
}

void SimulationDisplay::Deinitialize()
{
	TwTerminate();
}
