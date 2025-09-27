#include "airbornestate.h"
#include "../events.h"
#include "../gameobject.h"
#include "../resources.h"
#include "../commands.h"

#include "idlestate.h"

void AirborneState::onEnter(GameObject &owner)
{
	Resources &res = Resources::getInstance();

	owner.getCommandDispatch().submit(
		Command{ .id = Commands::SetAnimation, .param = res.ANIM_PLAYER_RUN });

	Command setTexCmd;
	setTexCmd.id = Commands::SetTexture;
	setTexCmd.param.asPtr = res.texRun;
	owner.getCommandDispatch().submit(setTexCmd);

	Command setGroundedCmd;
	setGroundedCmd.id = Commands::SetGrounded;
	setGroundedCmd.param.asBool = false;
	owner.getCommandDispatch().submit(setGroundedCmd);
}

void AirborneState::onExit(GameObject &owner)
{
}

void AirborneState::onEvent(GameObject &owner, int eventId)
{
	switch (eventId)
	{
		case static_cast<int>(Events::landed):
		{
			owner.getController()->changeState(IdleState::instance());

			Command setGroundedCmd;
			setGroundedCmd.id = Commands::SetGrounded;
			setGroundedCmd.param.asBool = true;
			owner.getCommandDispatch().submit(setGroundedCmd);

			break;
		}
	}
}
