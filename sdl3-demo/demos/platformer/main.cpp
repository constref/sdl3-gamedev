#include <SDL3/SDL_main.h>
#include <iostream>
#include "external.h"
#include <config.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;

int main(int argc, char *argv[])
{
	bool awaitDebugger = false;
	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			if (strcmp(argv[i], "--debug"))
			{
				std::cout << "Please attach a debugger to the running process" << std::endl;
				while (!IsDebuggerPresent())
				{
					Sleep(100);
				}
			}
		}
	}
	if (Config::IsStandaloneMode())
	{
		StartAppStandalone();
	}
	else
	{
		const int editorPID = atoi(argv[argc - 1]);
		StartAppTooling(editorPID, 512, 288, 1920, 1080);
	}
	return 0;
}
