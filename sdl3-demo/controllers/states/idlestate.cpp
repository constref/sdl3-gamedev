#include "../../gameobject.h"
#include "../../commands.h"
#include "../../resources.h"
#include "../../events.h"

#include "idlestate.h"
#include "runningstate.h"

void IdleState::onEnter(GameObject &owner)
{
	Resources &res = Resources::getInstance();
	owner.getCommandDispatch().submit(
		Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_IDLE });

	Command setTexCmd;
	setTexCmd.id = Commands::SetTexture;
	setTexCmd.param.asPtr = res.texIdle;
	owner.getCommandDispatch().submit(setTexCmd);
}

void IdleState::onExit(GameObject &owner)
{
}

void IdleState::onEvent(GameObject &owner, int eventId)
{
	if (eventId == static_cast<int>(Events::run))
	{
		owner.getController()->changeState(RunningState::instance());
	}
}
