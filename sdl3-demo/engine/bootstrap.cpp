#include "bootstrap.h"
#include <iostream>
#include <cassert>
#include <config.h>
#include <engine.h>
#include <tooling/engineworker.h>

#include "tooling/editorlauncher.h"

#ifdef EXECUTION_MODE_TOOLING
#endif

Bootstrap::Bootstrap(int argc, char *argv[])
{
	m_argc = argc;
	m_argv = argv;
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
			}
		}
	}
}

int Bootstrap::exec(std::unique_ptr<Application> application, int logW, int logH, int width, int height) const
{
	if constexpr (Config::IsStandaloneMode())
	{
		Engine engine(std::move(application));
		if (!engine.initialize(logW, logH, width, height, nullptr))
		{
			return 1;
		}
		engine.run();
	}
	else
	{
		EditorLauncher::exec(std::move(application), m_argc, m_argv);
	}

	return 0;
}
