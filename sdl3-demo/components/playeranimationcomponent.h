#pragma once

#include "animationcomponent.h"
#include "../events.h"

class PlayerAnimationComponent : public AnimationComponent
{
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN = 1;
	const int ANIM_PLAYER_SLIDE = 2;
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;

public:
	PlayerAnimationComponent(GameObject &owner, const std::vector<Animation> &animations) : AnimationComponent(owner, animations)
	{
		setAnimation(ANIM_PLAYER_IDLE);
	}

	//void eventHandler(int eventId) override
	//{
	//	switch (eventId)
	//	{
	//		case static_cast<int>(Events::run):
	//		{
	//			setAnimation(ANIM_PLAYER_RUN);
	//			break;
	//		}
	//	}
	//}
};