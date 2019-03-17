#include "ConfigReader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>


ConfigReader::ConfigReader()
{
}


ConfigReader::~ConfigReader()
{
}

ConfigData ConfigReader::Read()
{
	ConfigData cd = {};

	std::ifstream cf(_fileName.c_str());
	std::string word;

	if (cf.is_open())
	{
		while (cf >> word)
		{
			if (word == "BallCount")
			{
				cf >> cd.ballCount;
			}
			else if (word == "BallStartHeight")
			{
				cf >> cd.ballStartHeight;
			}
			else if (word == "ArenaDiameter")
			{
				cf >> cd.arenaDiameter;
			}
			else if (word == "Friction")
			{
				cf >> cd.frictionalForce;
			}
			else if (word == "Elasticity")
			{
				cf >> cd.elasticity;
			}
			else if (word == "PeerAddress")
			{
				std::string adr;
				cf >> adr;

				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				cd.peerAddress = converter.from_bytes(adr);
			}
		}
		cf.close();
	}

	return cd;
}
