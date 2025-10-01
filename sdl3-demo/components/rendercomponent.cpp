#include "rendercomponent.h"
#include <SDL3/SDL.h>

#include "../gameobject.h"
#include "../sdlstate.h"
#include "../gamestate.h"
#include "../resources.h"
#include "../framecontext.h"
#include "../commands.h"
#include "../coresubjects.h"

RenderComponent::RenderComponent(GameObject &owner, SDL_Texture *texture, float width, float height)
	: Component(owner), flashTimer(0.05f)
{
	this->texture = texture;
	shouldFlash = false;
	this->width = width;
	this->height = height;
	this->frameNumber = 1;
	direction = 1;

	// receive frame number updates from the animation component
	//if (animComponent)
	//{
	//	animComponent->currentFrameChanged.addObserver([this](int frame) {
	//		this->frameNumber = frame;
	//		});
	//}
	//if (inputComponent)
	//{
	//	inputComponent->directionUpdate.addObserver([this](float direction) {
	//		if (direction)
	//		{
	//			this->direction = direction;
	//		}
	//		});
	//}
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
		.x = owner.getPosition().x - ctx.gs.mapViewport.x,
		.y = owner.getPosition().y - ctx.gs.mapViewport.y,
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

	if (ctx.gs.debugMode)
	{
		if (owner.isDebugHighlight())
		{
			owner.setDebugHighlight(false);
			SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_BLEND);

			SDL_SetRenderDrawColor(ctx.state.renderer, 255, 0, 0, 150);
			SDL_RenderFillRect(ctx.state.renderer, &dst);

			SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_NONE);
		}
		/*
			SDL_FRect rectA{
				.x = owner.position.x + owner.collider.x - ctx.gs.mapViewport.x,
				.y = owner.position.y + owner.collider.y,
				.w = owner.collider.w,
				.h = owner.collider.h
			};
			SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_BLEND);

			SDL_SetRenderDrawColor(ctx.state.renderer, 255, 0, 0, 150);
			SDL_RenderFillRect(ctx.state.renderer, &rectA);
			SDL_FRect sensor{
				.x = owner.position.x + owner.collider.x - ctx.gs.mapViewport.x,
				.y = owner.position.y + owner.collider.y + owner.collider.h,
				.w = owner.collider.w, .h = 1
			};
			SDL_SetRenderDrawColor(ctx.state.renderer, 0, 0, 255, 150);
			SDL_RenderFillRect(ctx.state.renderer, &sensor);

			SDL_SetRenderDrawBlendMode(ctx.state.renderer, SDL_BLENDMODE_NONE);
		*/
	}
}

void RenderComponent::onAttached(SubjectRegistry &registry)
{
	owner.getCommandDispatch().registerCommand(Commands::SetTexture, this);
}

void RenderComponent::onCommand(const Command &command)
{
	if (command.id == Commands::SetTexture)
	{
		SDL_Texture *texture = static_cast<SDL_Texture *>(command.param.asPtr);
		setTexture(texture);
	}
}

void RenderComponent::registerObservers(SubjectRegistry &registry)
{
	registry.addObserver<int>(CoreSubjects::CURRENT_ANIMATION_FRAME, [this](const int &currentFrame) {
		this->frameNumber = currentFrame;
	});

	registry.addObserver<float>(CoreSubjects::DIRECTION, [this](const float &direction) {
		if (direction)
		{
			this->direction = direction;
		}
	});
}

void RenderComponent::setTexture(SDL_Texture *texture)
{
	this->texture = texture;
}
