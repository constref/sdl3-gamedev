#pragma once

#include <gameobject.h>
#include <resources.h>
#include <sdlstate.h>
#include <components/animationcomponent.h>
#include <components/rendercomponent.h>
#include <components/inputcomponent.h>
#include <components/physicscomponent.h>
#include <components/collisioncomponent.h>
#include <components/basiccameracomponent.h>

#include "components/playercontrollercomponent.h"

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
