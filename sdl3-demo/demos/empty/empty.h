#pragma once

#include <application.h>

class Empty : public Application
{
public:
	bool initialize(Services &services, SDLState &state) override;
	void start(Services &services, SDLState &state) override;
};
