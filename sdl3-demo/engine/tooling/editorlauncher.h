#pragma once

#include <memory>
#include <application.h>

class EditorLauncher
{
public:
   static int exec(std::unique_ptr<Application> app, int argc, char *argv[]);
};
