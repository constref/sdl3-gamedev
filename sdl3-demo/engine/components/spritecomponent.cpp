#include "spritecomponent.h"

#include <node.h>
#include <sdlstate.h>
#include <framecontext.h>
#include <messaging/events.h>

SpriteComponent::SpriteComponent(Node &owner, SDL_Texture *texture, float width, float height)
	: Component(owner, FrameStage::Render), flashTimer(0.05f)
{
	this->texture = texture;
	shouldFlash = false;
	this->width = width;
	this->height = height;
	scale = glm::vec2(1.0f, 1.0f);
	rotation = 0;
	frameNumber = 1;
	viewportPos = { 0, 0 };
	followViewport = 1;
	flipMode = SDL_FLIP_NONE;
	viewportSize = { 0, 0 };
}
