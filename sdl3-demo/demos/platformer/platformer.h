#pragma once

#include <ghandle.h>

struct SDLState;
struct FrameContext;
class World;

class Platformer
{
	GHandle hRoot;
	GHandle hPlayer;

	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

public:
	Platformer();

	GHandle getPlayerHandle() const { return hPlayer; }
	bool initialize(SDLState &state);
	void cleanup();
	void onStart();
	void update(const FrameContext &ctx);

	auto getRoot() const
	{
		return hRoot;
	}
};
