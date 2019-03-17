#pragma once

#include <memory>
#include <vector>

#include "GraphicsDevice.h"
#include "SimulationDisplay.h"
#include "Timer.h"
#include "Ball.h"
#include "Box.h"
#include "Camera.h"
#include "PhysicsManager.h"
#include "Influencer.h"
#include "Arena.h"
#include "NetworkManager.h"
#include "ConfigReader.h"

#include <AntTweakBar.h>

#include <CommonStates.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include <Keyboard.h>
#include <Mouse.h>


class SimulationSystem
{
private:
	// DISPLAY
	SimulationDisplay _display;
	HWND _displayHandle;

	// ANTTWEAK
	TwBar* _debugBar;

	// CONFIG
	ConfigReader _configReader;

	// TIME
	float _timeScale = 1.f;
	float _timeScaleRate = 0.2f;
	bool _timePaused = false;

	float _frequencyScaleRate = 5.f;

	Timer _physicsTimer;
	float _physicsTimeStep = 0.f;
	double _physicsFrequency = 60;

	Timer _graphicsTimer;
	float _graphicsTimeStep;
	double _graphicsFrequency = 60;

	Timer _controlTimer;

	bool _closing = false;

	// CONTROLS
	int _mouseX;
	int _mouseY;
	float _mouseMoveMultiplier = 0.1f;

	bool _mouseForceCancelled = false;

	std::unique_ptr<DirectX::Mouse> _mouse;
	DirectX::Mouse::ButtonStateTracker _mouseTracker;

	std::unique_ptr<DirectX::Keyboard> _keyboard;
	DirectX::Keyboard::KeyboardStateTracker _keyboardTracker;

	// CAMERA
	float _cameraMovementSpeed = 0.1f;
	float _cameraZoomSpeed = 0.01f;

	Camera _camera;

	// ARENA	
	float _arenaFloorHeight = 2.f;
	float _arenaDiameter = 150.f;
	float _arenaWallHeight = 20.f;

	Arena _arena;

	// BALLS
	float _ballStartY = 10.f;
	float _ballDiameter = 0.5f;
	float _ballGridSeperation = 0.5f;
	unsigned _ballCount = 10;
	unsigned _ballsOwned = 0;

	std::vector<Ball> _balls;
	std::mutex _ballsGraphicsLock;
	std::mutex _ballsPhysicsLock;
	std::mutex _ballsResetLock;

	// INFLUENCERS
	float _influencerDiameter = 16.f;
	float _influencerRingThickness = 0.05f;
	float _influencerMarkerDiameter = 0.25f;
	float _influencerMarkerRingThickness = 0.02f;
	float _influcenceRate = 2.f;

	float _sightRegionDiameter = 30.f;

	Influencer _influencer;
	Influencer _peerInfluencer;

	// PHYSICS
	PhysicsManager _physicsManager;

	// NETWORKING
	NetworkManager _networkManager;

	// GRAPHICS RESOURCES
	std::shared_ptr<GraphicsDevice> _graphicsDevice;
	std::unique_ptr<DirectX::CommonStates> _commonStates;

	std::unique_ptr<DirectX::GeometricPrimitive> _ballModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _arenaModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _arenaEnclosureModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _influencerModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _influencerRingModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _influencerMarkerModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _influencerMarkerRingModel;
	std::unique_ptr<DirectX::GeometricPrimitive> _sightRegionModel;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _ballLightTx;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _ballMediumTx;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _ballHeavyTx;

	DirectX::XMVECTORF32 _influencerColor;
	DirectX::XMVECTORF32 _peerInfluencerColor;

	DirectX::SimpleMath::Matrix _world;
	DirectX::SimpleMath::Matrix _proj;

	std::vector<char> ReadFile(char* fileName);

	// Update
	void Setup();
	void Reset();

	void CheckControls(float deltaTime);
	void Draw();

	static void PhysicsLoop(SimulationSystem*);

	static void TW_CALL TwSetTimeScale(const void *value, void *clientData)
	{
		auto sys = static_cast<SimulationSystem *>(clientData);
		sys->_timeScale = *static_cast<const float *>(value);
		sys->_networkManager.TimeScaleChanged();
	}

	static void TW_CALL TwGetTimeScale(void *value, void *clientData)
	{
		*static_cast<float *>(value) = static_cast<SimulationSystem *>(clientData)->_timeScale;
	}

public:
	SimulationSystem(HINSTANCE instanceHandle);
	~SimulationSystem();

	bool Initialize();

	int Run(int nCmdShow);

};

