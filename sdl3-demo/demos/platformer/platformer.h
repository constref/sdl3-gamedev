#pragma once

#include <nodehandle.h>

struct SDLState;
struct FrameContext;
class Services;

class Platformer
{
	NodeHandle hRoot;
	NodeHandle hPlayer;

	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

public:
	Platformer();

	NodeHandle getPlayerHandle() const { return hPlayer; }
	bool initialize(Services &services, SDLState &state);
	void cleanup();
	void onStart();
	void update();

	auto getRoot() const
	{
		return hRoot;
	}
};
