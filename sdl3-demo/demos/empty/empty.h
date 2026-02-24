#pragma once

#include <application.h>

class Empty : public Application
{

public:
	Empty();

	// Inherited via Application
	bool initialize(Services &services, SDLState &state) override;
	void start(Services &services, SDLState &state) override;
};
