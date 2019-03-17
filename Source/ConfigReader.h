#pragma once
#include <string>

struct ConfigData
{
	unsigned ballCount;
	float ballStartHeight;
	float arenaDiameter;
	float frictionalForce;
	float elasticity;
	std::wstring peerAddress;
};

class ConfigReader
{
private:
	std::wstring _fileName = L"Simulation.cfg";

public:
	ConfigReader();
	~ConfigReader();

	ConfigData Read();
};

