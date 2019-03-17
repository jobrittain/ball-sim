#pragma once

//#include "Ball.h"
#include <vector>
#include "Win32.h"
#include <WinSock2.h>
#include <thread>
#include "Timer.h"
#include "Ball.h"
#include "Influencer.h"

class NetworkManager
{
private:

	Timer _networkTimer;
	float _networkTimeStep;
	double _networkFrequency = 100;

	bool* _timePaused;
	float* _timeScale;
	std::vector<Ball>* _balls;
	unsigned *_ballsOwned;

	std::mutex* _ballsPhysicsLock;
	std::mutex* _ballsGraphicsLock;

	Influencer* _influencer;
	Influencer* _peerInfluencer;

	std::thread _listenThread;
	std::thread _talkThread;
	std::thread _recieveThread;

	bool _pausedChanged = false;
	bool _scaleChanged = false;
	bool _isClosing = false;

	bool _isHost = false;
	unsigned short _port = 9171U;

	WSADATA _wsaData;

	SOCKET _talkSocketInstance;
	SOCKET _listenSocketInstance;
	SOCKET _chosenSocket;

	std::wstring _peerAddress = L"127.0.0.1";

	static void StartNetwork(NetworkManager*);

	static void TalkLoop(NetworkManager*);
	static void RecieveLoop(NetworkManager*);

public:

	NetworkManager();
	~NetworkManager();

	bool Initialize();
	void Run();
	void Deinitialize();

	void PauseStateChanged();
	void TimeScaleChanged();
	void IncreaseNetworkFrequency(float quantity);
	void DecreaseNetworkFrequency(float quantity);

	bool IsHost();

	void SetTimeScale(float& timeScale);
	void SetTimePauseState(bool& pauseState);
	void SetBalls(std::vector<Ball>& balls, 
		std::mutex& ballsPhysicsLock, 
		std::mutex& ballsGraphicsLock,
		unsigned& ballsOwned);
	void SetInfluencers(Influencer& influencer, Influencer& peerInfluencer);
	void SetPeerAddress(const std::wstring& peerAddress);

	void RegisterTwBarVariables(TwBar* bar)
	{
		_networkTimer.RegisterTwBarCallbacks(bar, "Network Frequency");
		TwAddVarRO(bar, "Network Time Step", TW_TYPE_FLOAT, &_networkTimeStep, "");
	}

};

