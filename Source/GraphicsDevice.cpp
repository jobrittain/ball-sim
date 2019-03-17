#include "GraphicsDevice.h"
#include <vector>

GraphicsDevice::GraphicsDevice()
{
}

void GraphicsDevice::Initialize(const HWND windowHandle, const FLOAT screenWidth, const FLOAT screenHeight)
{
	// CREATE DEVICE AND SWAPCHAIN

	DXGI_SWAP_CHAIN_DESC scd = {};

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = static_cast<UINT>(screenWidth);
	scd.BufferDesc.Height = static_cast<UINT>(screenHeight);
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = windowHandle;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		NULL,
		nullptr,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		_swapChain.GetAddressOf(),
		_device.GetAddressOf(),
		nullptr,
		_deviceContext.GetAddressOf());

	// CREATE DEPTH BUFFER

	D3D11_TEXTURE2D_DESC bufferTexDes = {};

	bufferTexDes.Width = static_cast<UINT>(screenWidth);
	bufferTexDes.Height = static_cast<UINT>(screenHeight);
	bufferTexDes.ArraySize = 1;
	bufferTexDes.MipLevels = 1;
	bufferTexDes.SampleDesc.Count = 4;
	bufferTexDes.Format = DXGI_FORMAT_D32_FLOAT;
	bufferTexDes.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D *pDepthBuffer;
	_device->CreateTexture2D(&bufferTexDes, nullptr, &pDepthBuffer);

	// CREATE DEPTH STENCIL

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};

	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	_device->CreateDepthStencilView(pDepthBuffer, &dsvd, &_depthBuffer);
	pDepthBuffer->Release();

	// SETUP THE RENDER TARGET

	ID3D11Texture2D *pBackBuffer;
	_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
	{
		_device->CreateRenderTargetView(pBackBuffer, nullptr, _renderTargetView.GetAddressOf());
	}
	pBackBuffer->Release();

	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthBuffer.Get());

	// SET THE VIEWPORT

	D3D11_VIEWPORT viewport = {};

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = screenWidth;
	viewport.Height = screenHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	_deviceContext->RSSetViewports(1, &viewport);

	// INITIALIZE COM OBJECTS

	CoInitialize(nullptr);
}

void GraphicsDevice::UpdateScreenSize(const FLOAT screenWidth, const FLOAT screenHeight)
{
	D3D11_VIEWPORT viewport = {};

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = screenWidth;
	viewport.Height = screenHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	_deviceContext->RSSetViewports(1, &viewport);
}

void GraphicsDevice::Clear()
{
	_deviceContext->ClearRenderTargetView(
		_renderTargetView.Get(),
		DirectX::Colors::Black);

	_deviceContext->ClearDepthStencilView(
		_depthBuffer.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f, 0);
}

void GraphicsDevice::Swap()
{
	_swapChain->Present(0, 0);
}
