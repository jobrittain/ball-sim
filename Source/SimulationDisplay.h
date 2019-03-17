#pragma once
#include <memory>
#include "Window.h"
#include "GraphicsDevice.h"

const float InitialScreenWidth = 1280.f;
const float InitialScreenHeight = 720.f;

class SimulationSystem;

class SimulationDisplay : Window
{
private:
	HINSTANCE _instanceHandle;
	HWND _windowHandle;
	FLOAT _screenWidth;
	FLOAT _screenHeight;

	SimulationSystem * _simulationSystem;
	std::shared_ptr<GraphicsDevice> _graphicsDevice;

	LRESULT CALLBACK WndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) override;

public:
	SimulationDisplay(HINSTANCE instanceHandle);
	~SimulationDisplay();

	HWND Initialize(SimulationSystem * const simulationSystem);
	void Deinitialize();
	std::shared_ptr<GraphicsDevice> GetAssociatedDevice() { return _graphicsDevice; }
	void Show()
	{
		ShowWindow(_windowHandle, SW_SHOW);
	}

	float GetWidth()
	{
		return _screenWidth;
	}

	float GetHeight()
	{
		return _screenHeight;
	}
};

