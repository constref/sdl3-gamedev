#pragma once

#include <string>
#include <memory>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <application.h>

class Bootstrap
{
	bool awaitDebugger = false;
	int editorPID = 0;
	std::string editorUrl;

public:
	Bootstrap(int argc, char *argv[]);
	int exec(std::unique_ptr<Application> application, int logW, int logH, int width, int height);
};
