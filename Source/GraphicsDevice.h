#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXColors.h>

class GraphicsDevice
{
private:

	Microsoft::WRL::ComPtr<ID3D11Device> _device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;

public:
	GraphicsDevice();
	
	void Initialize(const HWND windowHandle, const FLOAT screenWidth, const FLOAT screenHeight);
	
	void UpdateScreenSize(const FLOAT screenWidth, const FLOAT screenHeight);

	void Clear();
	void Swap();

	ID3D11Device* Device() const
	{ 
		return _device.Get();
	}
	
	ID3D11DeviceContext* DeviceContext() const
	{ 
		return _deviceContext.Get();
	}
};