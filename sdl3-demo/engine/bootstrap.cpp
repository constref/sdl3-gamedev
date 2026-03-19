#include "bootstrap.h"
#include <iostream>
#include <cassert>
#include <config.h>
#include <engine.h>
#include <tooling/engineworker.h>

Bootstrap::Bootstrap(int argc, char *argv[])
{
	if constexpr (Config::IsToolingMode())
	{
		if (argc > 1)
		{
			for (int i = 0; i < argc; ++i)
			{
				if (strcmp(argv[i], "--debug") == 0)
				{
					std::cout << "Please attach a debugger to the running process" << std::endl;
					while (!IsDebuggerPresent())
					{
						Sleep(100);
					}
				}
				else if (strcmp(argv[i], "--pid") == 0)
				{
					if (i < argc)
					{
						editorPID = atoi(argv[i + 1]);
					}
				}
				else if (strcmp(argv[i], "--url") == 0)
				{
					if (i < argc)
					{
						url = std::string(argv[i + 1]);
					}
				}
			}
		}
		assert(editorPID != 0 && "Process ID for tooling needs to be provided via --pid <PID>");
		assert(url.length() > 0 && "No URL provided via --url <URL> to connect the tooling");
	}
}

int Bootstrap::exec(std::unique_ptr<Application> application, int logW, int logH, int width, int height)
{
	if constexpr (Config::IsStandaloneMode())
	{
		Engine engine(std::move(application));
		if (!engine.initialize(logW, logH, width, height))
		{
			return 1;
		}
		engine.run();
	}
	else
	{
		EngineWorker worker(std::move(application), editorPID, url);
		worker.start();
	}

	return 0;
}