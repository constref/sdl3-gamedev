#include "rendercomponent.h"
#include <SDL3/SDL.h>

#include <node.h>
#include <sdlstate.h>
#include <framecontext.h>
#include <messaging/commands.h>
#include <messaging/commanddispatcher.h>

RenderComponent::RenderComponent(Node &owner, SDL_Texture *texture, float width, float height)
	: Component(owner, ComponentStage::Render), flashTimer(0.05f)
{
	this->texture = texture;
	shouldFlash = false;
	this->width = width;
	this->height = height;
	this->frameNumber = 1;
	direction = 1;
	followViewport = 1;
	mapViewportPos = { 0, 0 };
	mapViewportSize = { 0, 0 };

	owner.getCommandDispatcher().registerHandler<SetAnimationCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateDirectionCommand>(this);
	owner.getCommandDispatcher().registerHandler<FrameChangeCommand>(this);
	owner.getCommandDispatcher().registerHandler<UpdateViewportCommand>(this);
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

void RenderComponent::onCommand(const SetAnimationCommand &dp)
{
	setTexture(dp.getTexture());
}

void RenderComponent::onCommand(const UpdateDirectionCommand &dp)
{
	if (dp.getDirection() != 0)
	{
		direction = dp.getDirection();
	}
}

void RenderComponent::onCommand(const FrameChangeCommand &dp)
{
	frameNumber = dp.getFrameNumber();
}

void RenderComponent::onCommand(const UpdateViewportCommand &dp)
{
	mapViewportPos.x = dp.getPosition().x;
	mapViewportPos.y = dp.getPosition().y;
	mapViewportSize.x = dp.getSize().x;
	mapViewportSize.y = dp.getSize().y;
}
