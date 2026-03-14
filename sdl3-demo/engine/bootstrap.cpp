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
				else if (strcmp(argv[i], "--editor-url") == 0)
				{
					if (i < argc)
					{
						editorUrl = std::string(argv[i + 1]);
					}
				}
			}
		}
		assert(editorPID != 0 && "Process ID for tooling needs to be provided via --pid <PID>");
		assert(editorUrl.length() > 0 && "No URL provided via --editor-url <URL> to connect the tooling");
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
		EngineWorker worker(std::make_unique<Engine>(std::move(application)), editorPID, editorUrl, logW, logH, width, height);
		worker.start();
	}

	return 0;
}