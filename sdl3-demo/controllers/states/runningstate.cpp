#include "../../commands.h"
#include "../../resources.h"
#include "../../events.h"
#include "../../gameobject.h"

#include "runningstate.h"
#include "idlestate.h"
#include "airbornestate.h"
#include "slidingstate.h"

void RunningState::onEnter(GameObject &owner)
{
	Resources &res = Resources::getInstance();

	owner.getCommandDispatch().submit(
		Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_RUN });

	Command setTexCmd;
	setTexCmd.id = Commands::SetTexture;
	setTexCmd.param.asPtr = res.texRun;
	owner.getCommandDispatch().submit(setTexCmd);
}
void RunningState::onExit(GameObject &owner)
{
}

void RunningState::onEvent(GameObject &owner, int eventId)
{
	if (eventId == static_cast<int>(Events::idle))
	{
		owner.getController()->changeState(IdleState::instance());
	}
	else if (eventId == static_cast<int>(Events::jump))
	{
		owner.getCommandDispatch().submit(Command{ .id = Commands::Jump });
		owner.getController()->changeState(AirborneState::instance());
	}
	else if (eventId == static_cast<int>(Events::falling))
	{
		owner.getController()->changeState(AirborneState::instance());
	}
	else if (eventId == static_cast<int>(Events::slide))
	{
		owner.getController()->changeState(SlidingState::instance());
	}
}