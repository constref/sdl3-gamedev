#include "rendercomponent.h"
#include <SDL3/SDL.h>

#include "../gameobject.h"
#include "../sdlstate.h"
#include "../resources.h"
#include "../framecontext.h"
#include "../messaging/messages.h"

SDL_FRect RenderComponent::mapViewport = { 0, 0, 0, 0 };

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

void RenderComponent::update(const FrameContext &ctx)
{
	//float srcX = owner.currentAnimation != -1
	//	? owner.animations[owner.currentAnimation].currentFrame() * width
	//	: (owner.spriteFrame - 1) * width;
	float srcX = (frameNumber - 1) * width;
	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = width,
		.h = height
	};

	SDL_FRect dst{
		.x = owner.getPosition().x - mapViewport.x * followViewport,
		.y = owner.getPosition().y - mapViewport.y * followViewport,
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

	//if (ctx.gs.debugMode)
	//{
	//	if (owner.isDebugHighlight())
	//	{
	//		owner.setDebugHighlight(false);
	//		SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_BLEND);

	//		SDL_SetRenderDrawColor(ctx.state.renderer, 255, 0, 0, 150);
	//		SDL_RenderFillRect(ctx.state.renderer, &dst);

	//		SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_NONE);
	//	}
	//}
}

void RenderComponent::onAttached(MessageDispatch &msgDispatch)
{
	msgDispatch.registerHandler<RenderComponent, SetAnimationMessage>(this);
	msgDispatch.registerHandler<RenderComponent, DirectionMessage>(this);
	msgDispatch.registerHandler<RenderComponent, FrameChangeMessage>(this);
	msgDispatch.registerHandler<RenderComponent, ViewportMessage>(this);
}

void RenderComponent::onMessage(const SetAnimationMessage &msg)
{
	setTexture(msg.getTexture());
}

void RenderComponent::onMessage(const DirectionMessage &msg)
{
	if (msg.getDirection() != 0)
	{
		direction = msg.getDirection();
	}
}

void RenderComponent::onMessage(const FrameChangeMessage &msg)
{
	frameNumber = msg.getFrameNumber();
}

void RenderComponent::onMessage(const ViewportMessage &msg)
{
	mapViewport.x = msg.getPosition().x;
	mapViewport.y = msg.getPosition().y;
	mapViewport.w = msg.getSize().x;
	mapViewport.h = msg.getSize().y;
}

void RenderComponent::setTexture(SDL_Texture *texture)
{
	this->texture = texture;
}
