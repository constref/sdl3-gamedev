#pragma once

#include <memory>
#include <application.h>

class Bootstrap
{
	int m_argc;
	char **m_argv;
	bool awaitDebugger = false;

public:
	Bootstrap(int argc, char *argv[]);
	int exec(std::unique_ptr<Application> application, int logW, int logH, int width, int height) const;
};
