#include "NetworkManager.h"

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <SimpleMath.h>

void Message(const char* message)
{
	std::cout << message << std::endl;
}

void ErrorMessage(const char* message)
{
	std::cerr << message << std::endl;
}

void NetworkManager::StartNetwork(NetworkManager* nm)
{
	nm->_networkTimeStep = 0.f;

	if (nm->_isHost)
	{
		if (listen(nm->_listenSocketInstance, 5) == SOCKET_ERROR)
		{
			std::cerr << "Listen failed with " << WSAGetLastError() << std::endl;
			return;
		}

		Message("Waiting for connection");

		nm->_chosenSocket = accept(nm->_listenSocketInstance, NULL, NULL);

		if (nm->_chosenSocket == INVALID_SOCKET)
		{
			std::cerr << "Accept failed with " << WSAGetLastError() << std::endl;
			return;
		}
		Message("Connection accepted");

	}
	else
	{
		nm->_chosenSocket = nm->_talkSocketInstance;
	}
	
	nm->_recieveThread = std::thread(RecieveLoop, nm);
	SetThreadAffinityMask(nm->_recieveThread.native_handle(), 0b10);

	TalkLoop(nm);

}

enum MessageType
{
	Pause = 0xA,
	TimeScale = 0xB,
	InfluencerData = 0xC,
	BallData = 0xD
};

using namespace DirectX;

void NetworkManager::TalkLoop(NetworkManager* nm)
{
	std::string str;

	std::vector<char> message;
	bool messageAvailable = false;
	char type;

	while (true)
	{
		nm->_networkTimer.Tick([&]
		{
			if (nm->_pausedChanged)
			{
				type = static_cast<char>(MessageType::Pause);
				message = std::vector<char>(sizeof(type) + sizeof(bool));

				message[0] = type;
				std::memcpy(&message[1], &(*nm->_timePaused), sizeof(bool));

				nm->_pausedChanged = false;
				messageAvailable = true;
			}
			else if (nm->_scaleChanged)
			{
				type = static_cast<char>(MessageType::TimeScale);
				message = std::vector<char>(sizeof(type) + sizeof(float));

				message[0] = type;
				std::memcpy(&message[1], &(*nm->_timeScale), sizeof(float));

				nm->_scaleChanged = false;
				messageAvailable = true;
			}

			if (messageAvailable)
			{
				if (send(nm->_chosenSocket, &message[0], static_cast<int>(message.size()), 0) == SOCKET_ERROR)
				{
					std::cerr << "Send failed with " << WSAGetLastError() << std::endl;
				}
				else
				{
					std::cout << "Message sent " << message.data() << std::endl;
				}
				messageAvailable = false;
			}
			
			{
				type = static_cast<char>(MessageType::InfluencerData);
				message = std::vector<char>(sizeof(type) + sizeof(XMVECTOR) + sizeof(float));

				XMVECTOR v = nm->_influencer->GetPosition();
				float f = nm->_influencer->GetForce();

				message[0] = type;
				std::memcpy(&message[1], &v, sizeof(XMVECTOR));
				std::memcpy(&message[1] + sizeof(XMVECTOR), &f, sizeof(float));

				if (send(nm->_chosenSocket, &message[0], static_cast<int>(message.size()), 0) == SOCKET_ERROR)
				{
					std::cerr << "Send failed with " << WSAGetLastError() << std::endl;
				}
				else
				{
					std::cout << "Message sent " << message.data() << std::endl;
				}
			}

			{
				std::lock_guard<std::mutex> lock(*nm->_ballsPhysicsLock);
				size_t ballsSize = nm->_balls->size();
				for (unsigned i = 0; i < ballsSize; i++)
				{
					Ball * b = &nm->_balls->at(i);

					if (b->ToBeSent())
					{
						// SEND BALL
						type = static_cast<char>(MessageType::BallData);
						message = std::vector<char>(sizeof(type) + sizeof(Ball));

						message[0] = type;
						std::memcpy(&message[1], &nm->_balls->at(i), sizeof(Ball));

						if (send(nm->_chosenSocket, &message[0], static_cast<int>(message.size()), 0) == SOCKET_ERROR)
						{
							std::cerr << "Ball send failed with " << WSAGetLastError() << std::endl;
						}
						else
						{
							std::cout << "Ball sent " << message.data() << std::endl;

							if (!b->IsContended() && !b->IsInSight())
							{
								b->SetOwnedState(false);
								*(nm->_ballsOwned) -= 1;
							}

							b->SetSendState(false);
						}
					}


				}
			}

			nm->_networkTimeStep = static_cast<float>(nm->_networkTimer.GetElapsedSeconds());
		});

		if (nm->_isClosing)
		{
			break;
		}
	}
}

void NetworkManager::RecieveLoop(NetworkManager * nm)
{
	bool typeEstablished = false;
	MessageType type;
	
	std::vector<char> message;

	while (true)
	{
		if (!typeEstablished)
		{
			char t;
			if (recv(nm->_chosenSocket, &t, 1, 0) == SOCKET_ERROR)
			{
				std::cerr << "Receive failed with " << WSAGetLastError() << std::endl;
				break;
			}

			type = static_cast<MessageType>(t);
			typeEstablished = true;
		}
		else
		{
			switch (type)
			{
			case MessageType::Pause:
				message = std::vector<char>(sizeof(bool));
				recv(nm->_chosenSocket, &message[0], sizeof(bool), 0);

				bool state;
				std::memcpy(&state, &message[0], sizeof(bool));

				*nm->_timePaused = state;

				break;
			case MessageType::TimeScale:
				message = std::vector<char>(sizeof(float));
				recv(nm->_chosenSocket, &message[0], sizeof(float), 0);

				float scale;
				std::memcpy(&scale, &message[0], sizeof(float));

				*nm->_timeScale = scale;

				break;
			case MessageType::InfluencerData:
				message = std::vector<char>(sizeof(XMVECTOR) + sizeof(float));
				recv(nm->_chosenSocket, &message[0], sizeof(XMVECTOR) + sizeof(float), 0);

				XMVECTOR position;
				float force;
				std::memcpy(&position, &message[0], sizeof(XMVECTOR));
				std::memcpy(&force, &message[0] + sizeof(XMVECTOR), sizeof(float));

				nm->_peerInfluencer->SetPosition(position);
				nm->_peerInfluencer->SetForce(force);
				nm->_peerInfluencer->SetOnlineState(true);

				break;
			case MessageType::BallData:
				message = std::vector<char>(sizeof(Ball));
				recv(nm->_chosenSocket, &message[0], sizeof(Ball), 0);

				Ball ball;
				std::memcpy(&ball, &message[0], sizeof(Ball));

				ball.SetSendState(false);
				ball.SetValidState(true);

				if (ball.IsContended())
				{
					ball.SetOwnedState(false);
				}
				else
				{
					*(nm->_ballsOwned) += 1;
				}

				nm->_balls->at(ball.GetID()) = ball;
				
				break;
			}
			typeEstablished = false;
		}

		if (nm->_isClosing)
		{
			break;
		}
	}
}

NetworkManager::NetworkManager()
{
}

NetworkManager::~NetworkManager()
{
}

void AllocateConsole()
{
	AllocConsole();
	FILE* out;
	FILE* in;
	freopen_s(&in, "conin$", "r", stdin);
	freopen_s(&out, "conout$", "w", stdout);
	freopen_s(&out, "conout$", "w", stderr);
}



bool NetworkManager::Initialize()
{
	AllocateConsole();

	_networkTimer.SetFixedTimeStep(true);
	_networkTimer.SetTargetFrequency(_networkFrequency);

	// Create version identifier
	WORD version = MAKEWORD(2, 2);

	// Startup windows sockets
	if (WSAStartup(version, &_wsaData))
	{
		ErrorMessage("Socket initialization failed");
		return false;
	}
	Message("Socket initialized");


	// TRY TO CONNECT

	// Create socket internet address info
	SOCKADDR_IN socketAddress = {};

	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(_port);

	InetPton(socketAddress.sin_family, _peerAddress.c_str(), &socketAddress.sin_addr);

	// Create transfer socket
	_talkSocketInstance = socket(socketAddress.sin_family, SOCK_STREAM, IPPROTO_TCP);

	if (_talkSocketInstance == INVALID_SOCKET)
	{
		ErrorMessage("Create socket failed");
		return false;
	}
	else if (connect(_talkSocketInstance, reinterpret_cast<sockaddr*>(&socketAddress), sizeof(sockaddr)) == SOCKET_ERROR)
	{
		// LISTENING FOR CONNECTIONS

		Message("Becoming host");

		_isHost = true;

		SOCKADDR_IN peer;
		peer.sin_family = AF_INET;
		peer.sin_port = htons(_port);
		peer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		// Create listening socket
		_listenSocketInstance = socket(AF_INET, SOCK_STREAM, 0);

		if (_listenSocketInstance == INVALID_SOCKET)
		{
			std::cerr << "Create socket failed" << std::endl;
			return false;
		}
		else if (bind(_listenSocketInstance, (sockaddr *)&peer, sizeof(peer)) == SOCKET_ERROR)
		{
			std::cerr << "Bind failed with " << WSAGetLastError() << std::endl;
			return false;
		}
	}


	return true;
}

void NetworkManager::Run()
{
	_listenThread = std::thread(&NetworkManager::StartNetwork, this);

	Message("network threads started");

	auto networkAffinity = 0b10;
	SetThreadAffinityMask(_listenThread.native_handle(), networkAffinity);

	Message("network threads affinity set");
}

void NetworkManager::Deinitialize()
{
	closesocket(_listenSocketInstance);
	closesocket(_talkSocketInstance);
	closesocket(_chosenSocket);

	_isClosing = true;
	

	if (_listenThread.joinable())
	{
		_listenThread.join();
	}
	if (_talkThread.joinable())
	{
		_talkThread.join();
	}

	// Cleanup windows sockets
	WSACleanup();
}

void NetworkManager::PauseStateChanged()
{
	_pausedChanged = true;
}

void NetworkManager::TimeScaleChanged()
{
	_scaleChanged = true;
}

void NetworkManager::IncreaseNetworkFrequency(float quantity)
{
	_networkFrequency += quantity;
	_networkTimer.SetTargetFrequency(_networkFrequency);
}

void NetworkManager::DecreaseNetworkFrequency(float quantity)
{
	_networkFrequency -= quantity;
	_networkTimer.SetTargetFrequency(_networkFrequency);
}

bool NetworkManager::IsHost()
{
	return _isHost;
}

void NetworkManager::SetTimeScale(float & timeScale)
{
	_timeScale = &timeScale;
}

void NetworkManager::SetTimePauseState(bool & pauseState)
{
	_timePaused = &pauseState;
}

void NetworkManager::SetBalls(std::vector<Ball>& balls, std::mutex& ballsPhysicsLock, std::mutex& ballsGraphicsLock, unsigned& ballsOwned)
{
	_balls = &balls;
	_ballsOwned = &ballsOwned;
	_ballsPhysicsLock = &ballsPhysicsLock;
	_ballsGraphicsLock = &ballsGraphicsLock;
}

void NetworkManager::SetInfluencers(Influencer & influencer, Influencer & peerInfluencer)
{
	_influencer = &influencer;
	_peerInfluencer = &peerInfluencer;
}

void NetworkManager::SetPeerAddress(const std::wstring& peerAddress)
{
	_peerAddress = peerAddress;
}
