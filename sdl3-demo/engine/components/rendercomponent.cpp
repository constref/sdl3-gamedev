#include "rendercomponent.h"
#include <SDL3/SDL.h>

#include "../gameobject.h"
#include "../sdlstate.h"
#include "../resources.h"
#include "../framecontext.h"
#include <messaging/datapumps.h>
#include <messaging/datadispatcher.h>

glm::vec2 RenderComponent::mapViewportPos = { 0, 0 };
glm::vec2 RenderComponent::mapViewportSize = { 0, 0 };

RenderComponent::RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height)
	: Component(owner, ComponentStage::Render), flashTimer(0.05f)
{
	this->texture = texture;
	shouldFlash = false;
	this->width = width;
	this->height = height;
	this->frameNumber = 1;
	direction = 1;
	followViewport = 1;
}

void RenderComponent::onAttached(DataDispatcher &dataDispatcher, EventDispatcher &eventDispatcher)
{
	dataDispatcher.registerHandler<RenderComponent, SetAnimationDPump>(this);
	dataDispatcher.registerHandler<RenderComponent, DirectionDPump>(this);
	dataDispatcher.registerHandler<RenderComponent, FrameChangeDPump>(this);
	dataDispatcher.registerHandler<RenderComponent, ViewportDPump>(this);
}

void RenderComponent::update(const FrameContext &ctx)
{
	float srcX = (frameNumber - 1) * width;
	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = width,
		.h = height
	};

	SDL_FRect dst{
		.x = owner.getPosition().x - mapViewportPos.x * followViewport,
		.y = owner.getPosition().y - mapViewportPos.y * followViewport,
		.w = width,
		.h = height
	};
	SDL_FlipMode flipMode = direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (!shouldFlash)
	{
		SDL_RenderTextureRotated(ctx.state.renderer, texture, &src, &dst, 0, nullptr, flipMode);
	}
	else
	{
		// flash object with a redish tint
		SDL_SetTextureColorModFloat(texture, 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(ctx.state.renderer, texture, &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);

		if (flashTimer.step(ctx.deltaTime))
		{
			shouldFlash = false;
		}
	}
}

void RenderComponent::onData(const SetAnimationDPump &dp)
{
	setTexture(dp.getTexture());
}

void RenderComponent::onData(const DirectionDPump &dp)
{
	if (dp.getDirection() != 0)
	{
		direction = dp.getDirection();
	}
}

void RenderComponent::onData(const FrameChangeDPump &dp)
{
	frameNumber = dp.getFrameNumber();
}

void RenderComponent::onData(const ViewportDPump &dp)
{
	mapViewportPos.x = dp.getPosition().x;
	mapViewportPos.y = dp.getPosition().y;
	mapViewportSize.x = dp.getSize().x;
	mapViewportSize.y = dp.getSize().y;
}
