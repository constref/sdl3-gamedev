#include "enemycomponent.h"
#include <messaging/messaging.h>
#include <resources.h>
#include <logger.h>
#include <node.h>
#include <glm/glm.hpp>

#include "../events.h"

EnemyComponent::EnemyComponent(Node &owner, EnemyType type)
	: Component(owner, FrameStage::Gameplay), type(type), damagedTimer(0.5f)
{
	state = EnemyState::idle;
}

	//const Resources &res = Resources::get();
	//if (type == EnemyType::creeper)
	//{
	//	timerDamaged.reset();
	//	addTimer(timerDamaged);

	//	damageSource = event.getSource();
	//	Node &sourceNode = services.world().getNode(damageSource);
	//	glm::vec2 sourceDir = glm::normalize(glm::vec2(sourceNode.getPosition().x - owner.getPosition().x, 0));
	//	owner.getCommandDispatcher().send<UpdateDirectionCommand>(UpdateDirectionCommand(sourceDir.x));

	//	// apply push force from projectile
	//	owner.sendCommand<AddImpulseCommand>(AddImpulseCommand{ glm::vec2(200 * -sourceDir.x, 220.0f) });

	//	switch (state)
	//	{
	//		case EnemyState::idle:
	//		{
	//			state = EnemyState::damaged;
	//			services.eventQueue().enqueue<AnimationPlayEvent>(owner.getHandle(), 0, res.ANIM_ENEMY_HIT, res.texEnemyHit, AnimationPlaybackMode::continuous);
	//			break;
	//		}
	//	}
	//}



