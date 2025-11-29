#include <SDL3/SDL.h>
#include <systems/spriterendersystem.h>
#include <components/spritecomponent.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <sdlstate.h>
#include <framecontext.h>
#include <systems/context/rendercontext.h>
#include <world.h>

SpriteRenderSystem::SpriteRenderSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<DirectionChangedEvent>(this);
	services.eventQueue().dispatcher.registerHandler<AnimationPlayEvent>(this);
}

void SpriteRenderSystem::update(Node &node)
{
	auto [sc] = getRequiredComponents(node);

	glm::vec2 size = sc->getSize();
	float srcX = (sc->getFrameNumber() - 1) * size.x;
	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = size.x,
		.h = size.y
	};

	SDL_FRect dst{
		.x = node.getPosition().x - RenderContext::shared().getCameraPosition().x * sc->getFollowViewport(),
		.y = node.getPosition().y - RenderContext::shared().getCameraPosition().y * sc->getFollowViewport(),
		.w = size.x * sc->getScale().x,
		.h = size.y * sc->getScale().y
	};

	// avoid drawing sprites outside of the logical viewport
	if (dst.x + dst.w > 0 && dst.x < SDLState::global().logW)
	{
		SDL_Renderer *renderer = SDLState::global().renderer;
		if (!sc->isShouldFlash())
		{
			if (sc->getParalaxFactor() == 0)
			{
				SDL_RenderTextureRotated(renderer, sc->getTexture(), &src, &dst, sc->getRotation(), nullptr, sc->getFlipMode());
			}
			else
			{
				SDL_FRect tiledDst = dst;
				tiledDst.x -= std::fmod(RenderContext::shared().getCameraPosition().x * sc->getParalaxFactor(), dst.w);
				tiledDst.x -= dst.w;
				for (int i = 0; i < 3; ++i)
				{
					SDL_RenderTextureRotated(renderer, sc->getTexture(), &src, &tiledDst, sc->getRotation(), nullptr, sc->getFlipMode());
					tiledDst.x += dst.w;
				}
			}
		}
		else
		{
			// flash object with a redish tint
			SDL_SetTextureColorModFloat(sc->getTexture(), 2.5f, 1.0f, 1.0f);
			SDL_RenderTextureRotated(renderer, sc->getTexture(), &src, &dst, sc->getRotation(), nullptr, sc->getFlipMode());
			SDL_SetTextureColorModFloat(sc->getTexture(), 1.0f, 1.0f, 1.0f);

			if (sc->getFlashTimer().step(FrameContext::dt()))
			{
				sc->setShouldFlash(false);
			}
		}
	}
}

void SpriteRenderSystem::onEvent(NodeHandle target, const AnimationPlayEvent &event)
{
	Node &node = services.world().getNode(target);
	if (node.isLinkedWith(this))
	{
		auto [sc] = getRequiredComponents(node);
		sc->setTexture(event.getTexture());
	}
}

void SpriteRenderSystem::onEvent(NodeHandle target, const DirectionChangedEvent &event)
{
	if (event.getDirection().x != 0)
	{
		Node &node = services.world().getNode(target);
		if (node.isLinkedWith(this))
		{
			auto [sc] = getRequiredComponents(node);
			sc->setFlipMode(event.getDirection().x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		}
	}
}
