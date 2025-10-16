#pragma once

#include <sdlstate.h>
#include <inputstate.h>
#include <gamestate.h>
#include <resources.h>
#include <framecontext.h>
#include <application.h>

template<Application AppType>
class Engine
{
	SDLState state;
	uint64_t prevTime = SDL_GetTicks();
	InputState inputState;
	AppType app;
	bool debugMode;
	bool running;

public:
	Engine(int logW, int logH) : state(1600, 900, logW, logH)
	{
		debugMode = false;
		running = true;
	}

	bool initialize()
	{
		if (state.initialize() && app.initialize(state))
		{
			return true;
		}
		return false;
	}

	void cleanup()
	{
		Resources::getInstance().unload();
		state.cleanup();
	}

	void run()
	{
		while (running)
		{
			SDL_Event event{ 0 };
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_EVENT_QUIT:
					{
						running = false;
						break;
					}
					case SDL_EVENT_WINDOW_RESIZED:
					{
						state.width = event.window.data1;
						state.height = event.window.data2;
						break;
					}
					case SDL_EVENT_KEY_DOWN:
					{
						inputState.setKeyState(event.key.scancode, true);
						inputState.addEvent(event.key.scancode, true);
						break;
					}
					case SDL_EVENT_KEY_UP:
					{
						inputState.setKeyState(event.key.scancode, false);
						inputState.addEvent(event.key.scancode, true);
						if (event.key.scancode == SDL_SCANCODE_F2)
						{
							debugMode = !debugMode;
						}
						else if (event.key.scancode == SDL_SCANCODE_F11)
						{
							state.fullscreen = !state.fullscreen;
							SDL_SetWindowFullscreen(state.window, state.fullscreen);
						}
						break;
					}
				}
			}

			// calculate deltaTime
			uint64_t nowTime = SDL_GetTicks();
			float deltaTime = (nowTime - prevTime) / 1000.0f;
			prevTime = nowTime;

			Resources &res = Resources::getInstance();
			FrameContext ctx(state, inputState, deltaTime);

			SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
			SDL_RenderClear(state.renderer);

			update(app.getRoot(), ctx);

			// calculate viewport position
			//gs.mapViewport.x = (gs.player()->getPosition().x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
			//gs.mapViewport.y = res.map->mapHeight * res.map->tileHeight - gs.mapViewport.h;

			//if (gs.debugMode)
			//{
				// display some debug info
				//SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
				//SDL_RenderDebugText(state.renderer, 5, 5,
				//	std::format("G: {} - dt: {}", gs.player()->getComponent<PhysicsComponent>()->isGrounded(), ctx.deltaTime).c_str());
			//}

			// swap buffers and present
			SDL_RenderPresent(state.renderer);
		}
	}

	void update(std::shared_ptr<GameObject> &obj, const FrameContext &ctx)
	{
		obj->update(ctx);

		auto &children = obj->getChildren();
		for (auto &child : children)
		{
			update(child, ctx);
		}
	}
};