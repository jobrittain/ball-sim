#include <iostream>
#include "Window.h"
#include "SimulationSystem.h"
#include <WinBase.h>
#include <vector>

int APIENTRY wWinMain(const HINSTANCE hInstance, const HINSTANCE, const LPWSTR, const int nCmdShow)
{
	// SET EXECUTABLE DIRECTORY AS CURRENT

	std::vector<wchar_t> exePathV = std::vector<wchar_t>(MAX_PATH);
	GetModuleFileName(NULL, &exePathV[0], MAX_PATH);

	const std::wstring exePathStr = exePathV.data();
	const size_t lastSlashIndex = exePathStr.rfind('\\');
	const std::wstring exeDirPath = exePathStr.substr(0, lastSlashIndex);

	SetCurrentDirectory(exeDirPath.c_str());
	
	// CREATE & RUN SIMULATION

	SimulationSystem system(hInstance);

	if (system.Initialize())
	{
		return system.Run(nCmdShow);
	}

	return EXIT_FAILURE;
}