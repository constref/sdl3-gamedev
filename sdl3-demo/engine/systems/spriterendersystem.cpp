#include <systems/spriterendersystem.h>
#include <SDL3/SDL.h>
#include <sdlstate.h>
#include <framecontext.h>

void SpriteRenderSystem::update(Node &node)
{
	auto [ rc ] = getRequiredComponents(node);

	glm::vec2 size = rc->getSize();
	float srcX = (rc->getFrameNumber() - 1) * size.x;
	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = size.x,
		.h = size.y
	};

	SDL_FRect dst{
		//.x = node.getPosition().x - mapViewportPos.x * followViewport,
		//.y = node.getPosition().y - mapViewportPos.y * followViewport,
		.x = node.getPosition().x * rc->getFollowViewport(),
		.y = node.getPosition().y - 350.0f * rc->getFollowViewport(),
		.w = size.x,
		.h = size.y
	};

	SDL_Renderer *renderer = SDLState::global().renderer;
	SDL_FlipMode flipMode = rc->getDirection().x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	
	if (!rc->isShouldFlash())
	{
		SDL_RenderTextureRotated(renderer, rc->getTexture(), &src, &dst, 0, nullptr, flipMode);
	}
	else
	{
		// flash object with a redish tint
		SDL_SetTextureColorModFloat(rc->getTexture(), 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(renderer, rc->getTexture(), &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(rc->getTexture(), 1.0f, 1.0f, 1.0f);

		if (rc->getFlashTimer().step(FrameContext::dt()))
		{
			rc->setShouldFlash(false);
		}
	}
}
