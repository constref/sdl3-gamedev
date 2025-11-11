#include "spritecomponent.h"
#include <SDL3/SDL.h>

#include <node.h>
#include <sdlstate.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/commanddispatcher.h>
#include <messaging/events.h>

SpriteComponent::SpriteComponent(Node &owner, SDL_Texture *texture, float width, float height)
	: Component(owner, FrameStage::Render), flashTimer(0.05f)
{
	this->texture = texture;
	this->shouldFlash = false;
	this->width = width;
	this->height = height;
	this->scale = glm::vec2(1.0f, 1.0f);
	this->frameNumber = 1;
	direction = 1;
	followViewport = 1;
	mapViewportPos = { 0, 0 };
	mapViewportSize = { 0, 0 };

	owner.getCommandDispatcher().registerHandler<UpdateViewportCommand>(this);
	owner.getEventDispatcher().registerHandler<AnimationPlayEvent>(this);
}

void SpriteComponent::update()
{
}

void SpriteComponent::onCommand(const UpdateViewportCommand &dp)
{
	mapViewportPos.x = dp.getPosition().x;
	mapViewportPos.y = dp.getPosition().y;
	mapViewportSize.x = dp.getSize().x;
	mapViewportSize.y = dp.getSize().y;
}

void SpriteComponent::onEvent(const AnimationPlayEvent &event)
{
	setTexture(event.getTexture());
}
