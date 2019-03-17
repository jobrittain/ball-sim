#include "Win32.h"
#include "SimulationSystem.h"

#include <AntTweakBar.h>
#include <iostream>
#include <fstream>
#include <DDSTextureLoader.h>
#include <random>
#include <Effects.h>
#include <thread>

using namespace DirectX;
using namespace DirectX::SimpleMath;

std::vector<char> SimulationSystem::ReadFile(char * fileName)
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		return buffer;
	}
	return std::vector<char>();
}

void SimulationSystem::CheckControls(float deltaTime)
{
	auto kb = _keyboard->GetState();
	_keyboardTracker.Update(kb);

	if (kb.Escape)
	{
		PostQuitMessage(EXIT_SUCCESS);
	}

	if (_keyboardTracker.pressed.R)
	{ 
		Reset();
	}

	if (_keyboardTracker.pressed.P)
	{
		_timePaused = !_timePaused;
		_networkManager.PauseStateChanged();
	}

	if (kb.O)
	{
		// increase physics freq
		_physicsFrequency += _frequencyScaleRate * deltaTime;
		_physicsTimer.SetTargetFrequency(_physicsFrequency);
	}
	else if (kb.L)
	{
		// decrease physics freq 
		_physicsFrequency -= _frequencyScaleRate * deltaTime;
		_physicsTimer.SetTargetFrequency(_physicsFrequency);
	}
	
	if (kb.I)
	{
		// increase graphics freq
		_graphicsFrequency += _frequencyScaleRate * deltaTime;
		_graphicsTimer.SetTargetFrequency(_graphicsFrequency);
	}
	else if (kb.K)
	{
		// decrease graphics freq
		_graphicsFrequency -= _frequencyScaleRate * deltaTime;
		_graphicsTimer.SetTargetFrequency(_graphicsFrequency);
	}

	if (kb.U)
	{
		// increase network freq
		_networkManager.IncreaseNetworkFrequency(_frequencyScaleRate * deltaTime);
	}
	else if (kb.J)
	{
		// decrease network freq
		_networkManager.DecreaseNetworkFrequency(_frequencyScaleRate * deltaTime);
	}

	if (kb.Y && _timeScale < 2.f)
	{
		_timeScale += _timeScaleRate * deltaTime;
		if (_timeScale > 2.f)
		{
			_timeScale = 2.f;
		}
		_networkManager.TimeScaleChanged();
	}
	else if (kb.H && _timeScale > 0.001f)
	{
		_timeScale -= _timeScaleRate * deltaTime;
		if (_timeScale < 0.001f)
		{
			_timeScale = 0.001f;
		}
		_networkManager.TimeScaleChanged();
	}

	if (kb.M)
	{
		_influencer.MoveUp(deltaTime);
		_camera.MoveUp(deltaTime);
	}
	else if (kb.N)
	{
		_influencer.MoveDown(deltaTime);
		_camera.MoveDown(deltaTime);
	}

	if (kb.W)
	{
		_influencer.MoveForwards(deltaTime);
		_camera.MoveForwards(deltaTime);
	}
	if (kb.S)
	{
		_influencer.MoveBackwards(deltaTime);
		_camera.MoveBackwards(deltaTime);
	}
	if (kb.A)
	{
		_influencer.MoveLeft(deltaTime);
		_camera.MoveLeft(deltaTime);
	}
	if (kb.D)
	{
		_influencer.MoveRight(deltaTime);
		_camera.MoveRight(deltaTime);
	}

	if (kb.Up)
	{
		_camera.ZoomIn(deltaTime);
	}
	else if (kb.Down)
	{
		_camera.ZoomOut(deltaTime);
	}



	auto mse = _mouse->GetState();

	if (_keyboardTracker.pressed.LeftControl || 
		_keyboardTracker.pressed.RightControl)
	{
		switch (mse.positionMode)
		{
		case Mouse::MODE_ABSOLUTE:
			_mouse->SetMode(Mouse::MODE_RELATIVE);
		case Mouse::MODE_RELATIVE:
			_mouse->SetMode(Mouse::MODE_ABSOLUTE);
		}
	}

	if (mse.positionMode == Mouse::MODE_RELATIVE)
	{
		if (mse.leftButton && mse.rightButton)
		{
			_influencer.CancelForce();
			_mouseForceCancelled = true;
		}

		if (mse.leftButton && !mse.rightButton &&
			_mouseForceCancelled == false)
		{
			_influencer.Attract(deltaTime);
		}

		if (mse.rightButton && !mse.leftButton &&
			_mouseForceCancelled == false)
		{
			_influencer.Repel(deltaTime);
		}

		if (!mse.rightButton && !mse.leftButton)
		{
			_mouseForceCancelled = false;
		}

		if (mse.scrollWheelValue)
		{
			_camera.MouseZoom(mse.scrollWheelValue);
			_mouse->ResetScrollWheelValue();
		}

		_camera.MouseMove(mse.x, mse.y);
		_influencer.MouseMove(mse.x, mse.y);
	}

	_mouseX = mse.x;
	_mouseY = mse.y;
}

SimulationSystem::SimulationSystem(HINSTANCE instanceHandle) :
	_display(instanceHandle)
{
}

SimulationSystem::~SimulationSystem()
{
}



enum TurnDirection
{
	Down, Right, Up, Left
};

bool SimulationSystem::Initialize()
{
	// SET MAIN THREAD AFFINITY TO CPU 0

	SetThreadAffinityMask(GetCurrentThread(), 0b1);

	// INITIALIZE DISPLAY

	_displayHandle = _display.Initialize(this);
	_graphicsDevice = _display.GetAssociatedDevice();

	auto dxDevice = _graphicsDevice->Device();
	auto dxContext = _graphicsDevice->DeviceContext();

	_commonStates = std::make_unique<DirectX::CommonStates>(_graphicsDevice->Device());

	// INITIALIZE NETWORK MANAGER

	ConfigData cd = _configReader.Read();

	_networkManager.SetPeerAddress(cd.peerAddress);
	_networkManager.Initialize();
	_networkManager.SetBalls(_balls, _ballsPhysicsLock, _ballsGraphicsLock, _ballsOwned);
	_networkManager.SetInfluencers(_influencer, _peerInfluencer);
	_networkManager.SetTimePauseState(_timePaused);
	_networkManager.SetTimeScale(_timeScale);

	// ANTTWEAK

	_debugBar = TwNewBar("Variables");

	TwDefine("Variables refresh=0.01 size='280 220'");

	_mouseX = 0;
	_mouseY = 0;

	TwAddVarRO(_debugBar, "Total Balls", TW_TYPE_UINT32, &_ballCount, "");
	TwAddVarRO(_debugBar, "Owned Balls", TW_TYPE_UINT32, &_ballsOwned, "");
	TwAddVarCB(_debugBar, "Time Scale", TW_TYPE_FLOAT, TwSetTimeScale, TwGetTimeScale, this, "min=0 max=2 step=0.001");
	
	_influencer.RegisterTwBarVariables(_debugBar);
	_physicsManager.RegisterTwBarVariables(_debugBar);

	_physicsTimer.SetFixedTimeStep(true);
	_physicsTimer.SetTargetFrequency(_physicsFrequency);
	_physicsTimer.RegisterTwBarCallbacks(_debugBar, "Physics Frequency");

	TwAddVarRO(_debugBar, "Physics Time Step", TW_TYPE_FLOAT, &_physicsTimeStep, "");

	_graphicsTimer.SetFixedTimeStep(true);
	_graphicsTimer.SetTargetFrequency(_graphicsFrequency);
	_graphicsTimer.RegisterTwBarCallbacks(_debugBar, "Graphics Frequency");

	TwAddVarRO(_debugBar, "Graphics Time Step", TW_TYPE_FLOAT, &_graphicsTimeStep, "");

	_networkManager.RegisterTwBarVariables(_debugBar);

	// SETUP MOUSE AND KEYBOARD

	_keyboard = std::make_unique<Keyboard>();
	_mouse = std::make_unique<Mouse>();
	_mouse->SetWindow(_displayHandle);
	_mouse->SetMode(Mouse::MODE_ABSOLUTE);

	// LOAD MODELS

	_ballModel = GeometricPrimitive::CreateSphere(
		dxContext, _ballDiameter);

	_influencerModel = GeometricPrimitive::CreateSphere(
		dxContext, _influencerDiameter);

	_influencerRingModel = GeometricPrimitive::CreateTorus(
		dxContext, _influencerDiameter, _influencerRingThickness);

	_influencerMarkerModel = GeometricPrimitive::CreateSphere(
		dxContext, _influencerMarkerDiameter);

	_influencerMarkerRingModel = GeometricPrimitive::CreateTorus(
		dxContext, _influencerMarkerDiameter, _influencerMarkerRingThickness);

	_sightRegionModel = GeometricPrimitive::CreateSphere(
		dxContext, _sightRegionDiameter);

	// LOAD COLOR & TEXTURES

	CreateDDSTextureFromFile(dxDevice,
		L"Textures\\light.dds",
		nullptr,
		_ballLightTx.GetAddressOf());

	CreateDDSTextureFromFile(dxDevice,
		L"Textures\\medium.dds",
		nullptr,
		_ballMediumTx.GetAddressOf());

	CreateDDSTextureFromFile(dxDevice,
		L"Textures\\heavy.dds",
		nullptr,
		_ballHeavyTx.GetAddressOf());

	// SETUP COMPONENTS

	Setup();

	return true;
}

void SimulationSystem::Setup()
{
	// RETRIEVE CONFIG

	ConfigData cd = _configReader.Read();

	_ballCount = cd.ballCount;
	_ballStartY = cd.ballStartHeight + (_arenaFloorHeight / 2) + (_ballDiameter / 2);
	_arenaDiameter = cd.arenaDiameter;

	// LOAD ARENA MODELS

	auto dxContext = _graphicsDevice->DeviceContext();

	_arenaModel = GeometricPrimitive::CreateCylinder(
		dxContext, _arenaFloorHeight, _arenaDiameter);

	_arenaEnclosureModel = GeometricPrimitive::CreateCylinder(
		dxContext, _arenaWallHeight, _arenaDiameter);

	// DECIDE INITIAL GROUND POSITION

	std::default_random_engine generator;

	Vector2 groundPosition;
	
	if (_networkManager.IsHost())
	{
		groundPosition.x = 0.f;
		groundPosition.y = 0.f;
	}
	else
	{
		std::uniform_real_distribution<float> influencerSpawnDist(-(_arenaDiameter / 4), (_arenaDiameter / 4));
		groundPosition.x = influencerSpawnDist(generator);
		groundPosition.y = influencerSpawnDist(generator);
	}

	// CAMERA

	_camera.SetPosition(Vector3(0.f + groundPosition.x, 20.f, 20.f + groundPosition.y));
	_camera.SetMovementSpeed(_cameraMovementSpeed);
	_camera.SetZoomSpeed(_cameraZoomSpeed);

	_world = Matrix::Identity;
	_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		_display.GetWidth() / _display.GetHeight(), 0.1f, 200.f);

	// INITIALIZE ARENA

	_arena.SetHeight(_arenaFloorHeight);
	_arena.SetRadius(_arenaDiameter / 2);
	_arena.SetPosition(Vector3(0, 0, 0));

	// INITIALIZE BALLS

	float ballSepW = _ballDiameter;
	float ballSepH = _ballDiameter;

	Vector3 curPos(0, _ballStartY, 0);

	float seperation = _ballGridSeperation + _ballDiameter;
	Vector3 up(0, 0, seperation);
	Vector3 down(0, 0, -seperation);
	Vector3 left(-seperation, 0, 0);
	Vector3 right(seperation, 0, 0);

	auto initCurCount = 0U;
	auto initCount = 1U;
	auto spawnedCount = 0U;

	TurnDirection dir = Down;


	std::uniform_int_distribution<unsigned> ballMassDist(0, 2);

	_ballsOwned = 0;

	while (spawnedCount < _ballCount)
	{
		for (unsigned y = 0; y < 2; ++y)
		{
			for (unsigned z = 0; z < initCount; ++z)
			{
				if (spawnedCount >= _ballCount)
				{
					break;
				}

				Ball ballInst = Ball();

				ballInst.SetPosition(curPos);
				if (_networkManager.IsHost())
				{
					ballInst.SetOwnedState(true);
					_ballsOwned++;
				}
				else
				{
					ballInst.SetOwnedState(false);
				}

				int roll = ballMassDist(generator);
				switch (roll)
				{
				case 0:
					ballInst.SetMass(BallMass::Light);
					break;
				case 1:
					ballInst.SetMass(BallMass::Medium);
					break;
				case 2:
					ballInst.SetMass(BallMass::Heavy);
					break;
				}

				_balls.push_back(ballInst);
				spawnedCount++;

				switch (dir)
				{
				case (Down):
					curPos += down;
					break;
				case (Right):
					curPos += right;
					break;
				case (Up):
					curPos += up;
					break;
				case (Left):
					curPos += left;
					break;
				}
			}

			switch (dir)
			{
			case (Down):
				dir = Right;
				break;
			case (Right):
				dir = Up;
				break;
			case (Up):
				dir = Left;
				break;
			case (Left):
				dir = Down;
				break;
			}
		}

		initCount++;
	}

	// INITIALIZE INFLUENCER

	if (_networkManager.IsHost())
	{
		_influencerColor = DirectX::Colors::White;
		_peerInfluencerColor = DirectX::Colors::Black;
	}
	else
	{
		_influencerColor = DirectX::Colors::Black;
		_peerInfluencerColor = DirectX::Colors::White;
	}
	_influencerColor.f[3] = 0.2f;
	_peerInfluencerColor.f[3] = 0.2f;

	_influencer.SetPosition(Vector3(groundPosition.x, _arena.GetPosition().y + (_arena.GetHeight() / 2), groundPosition.y));
	_influencer.SetMovementSpeed(_mouseMoveMultiplier);
	_influencer.SetInfluenceRate(_influcenceRate);
	_influencer.SetRadius(_influencerDiameter / 2);
	_influencer.SetSightRadius(_sightRegionDiameter / 2);

	_peerInfluencer.SetMovementSpeed(_mouseMoveMultiplier);
	_peerInfluencer.SetInfluenceRate(_influcenceRate);
	_peerInfluencer.SetRadius(_influencerDiameter / 2);
	_peerInfluencer.SetSightRadius(_sightRegionDiameter / 2);
	_peerInfluencer.SetOnlineState(false);

	// INITIALIZE PHYSICS MANAGER

	_physicsManager.SetFriction(cd.frictionalForce);
	_physicsManager.SetElasticity(cd.elasticity);
	_physicsManager.SetBalls(_balls, _ballDiameter / 2, _ballsPhysicsLock, _ballsResetLock);
	_physicsManager.SetArena(_arena);
	_physicsManager.SetInfluencer(_influencer, _influencerDiameter / 2);
	_physicsManager.SetPeerInfluencer(_peerInfluencer);
}

void SimulationSystem::Reset()
{
	{
		std::lock_guard<std::mutex> lockReset(_ballsResetLock);
		std::lock_guard<std::mutex> lockG(_ballsGraphicsLock);
		_balls.clear();
		Setup();
	}
}

int SimulationSystem::Run(int nCmdShow)
{
	ShowWindow(_displayHandle, nCmdShow);

	std::thread physicsThread(&SimulationSystem::PhysicsLoop, this);
	SetThreadAffinityMask(physicsThread.native_handle(), 0b11111100); // CPU 2 & 3

	_networkManager.Run();

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				_closing = true;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		_graphicsTimer.Tick([&]
		{
			Draw();

			_graphicsTimeStep = static_cast<float>(_graphicsTimer.GetElapsedSeconds());
		});

		_controlTimer.Tick([&] 
		{
			CheckControls(static_cast<float>(_controlTimer.GetElapsedSeconds()));
		});
	}

	if (physicsThread.joinable())
	{
		physicsThread.join();
	}

	_networkManager.Deinitialize();
	_display.Deinitialize();
	
	if (msg.message == WM_QUIT)
	{
		return static_cast<int>(msg.wParam);
	}
	return EXIT_SUCCESS;
}

void SimulationSystem::Draw()
{
	_graphicsDevice->Clear();

	auto cameraV = _camera.GetViewMatrix();

	Matrix mf = Matrix::CreateTranslation(_arena.GetPosition()) * _world;

	_arenaModel->Draw(mf, cameraV, _proj, Colors::SandyBrown);

	mf *= Matrix::CreateTranslation(Vector3(0, _arenaWallHeight / 2, 0));
	_arenaEnclosureModel->Draw(mf, cameraV, _proj, Colors::DarkGray, nullptr, false, [=]
	{
		auto ctx = _graphicsDevice->DeviceContext();

		ctx->RSSetState(_commonStates->CullClockwise());
	});


	{
		std::lock_guard<std::mutex> lock(_ballsGraphicsLock);

		for (Ball ball : _balls)
		{
			if (!ball.IsInSight() || !ball.IsValid())
			{
				continue;
			}

			Matrix mb = Matrix::Identity;

			if (ball.GetRotation().x < 0)
			{
				mb *= XMMatrixRotationRollPitchYaw(ball.GetRotation().x, ball.GetRotation().y, 0);
			}
			else
			{
				mb *= XMMatrixRotationRollPitchYaw(-ball.GetRotation().x, -ball.GetRotation().y, 0);
			}


			mb *= Matrix::CreateTranslation(ball.GetPosition()) * _world;

			DirectX::XMVECTORF32 _ballColor;

			if (!ball.IsContended())
			{
				_ballColor = DirectX::Colors::White;
			}
			else
			{
				_ballColor = DirectX::Colors::HotPink;
			}

			// Draw ball with texture according to mass
			switch (ball.GetMass())
			{
			case BallMass::Light:
				_ballModel->Draw(mb, cameraV, _proj, _ballColor, _ballLightTx.Get());
				break;
			case BallMass::Medium:
				_ballModel->Draw(mb, cameraV, _proj, _ballColor, _ballMediumTx.Get());
				break;
			case BallMass::Heavy:
				_ballModel->Draw(mb, cameraV, _proj, _ballColor, _ballHeavyTx.Get());
				break;
			}
		}
	}

	Matrix mi = Matrix::CreateTranslation(_influencer.GetPosition()) * _world;
	_influencerMarkerModel->Draw(mi, cameraV, _proj, DirectX::Colors::Purple);
	_influencerMarkerRingModel->Draw(mi, cameraV, _proj, DirectX::Colors::White);
	_influencerModel->Draw(mi, cameraV, _proj, _influencerColor);
	_influencerRingModel->Draw(mi, cameraV, _proj, DirectX::Colors::Purple);

	_sightRegionModel->Draw(mi, cameraV, _proj, _influencerColor);

	if (_peerInfluencer.IsOnline())
	{
		Matrix mip = Matrix::CreateTranslation(_peerInfluencer.GetPosition()) * _world;
		_influencerMarkerModel->Draw(mip, cameraV, _proj, DirectX::Colors::Purple);
		_influencerMarkerRingModel->Draw(mip, cameraV, _proj, DirectX::Colors::White);
		_influencerModel->Draw(mip, cameraV, _proj, _peerInfluencerColor);
		_influencerRingModel->Draw(mip, cameraV, _proj, DirectX::Colors::Purple);
		_sightRegionModel->Draw(mip, cameraV, _proj, _peerInfluencerColor);
	}

	TwDraw();

	_graphicsDevice->Swap();
}

void SimulationSystem::PhysicsLoop(SimulationSystem* ss)
{
	while (true)
	{
		ss->_physicsTimer.Tick([&]
		{
			if (!ss->_timePaused)
			{
				ss->_physicsManager.Update(ss->_physicsTimeStep * ss->_timeScale);
			}
		});

		ss->_physicsTimeStep = static_cast<float>(ss->_physicsTimer.GetElapsedSeconds());

		if (ss->_closing)
		{
			break;
		}
	}
}

