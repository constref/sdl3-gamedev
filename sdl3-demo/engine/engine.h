#pragma once

#include <sdlstate.h>
#include <inputstate.h>
#include <resources.h>
#include <framecontext.h>
#include <application.h>
#include <gameobject.h>
#include <world.h>

template<Application AppType>
class Engine
{
	SDLState state;
	uint64_t prevTime;
	InputState inputState;
	AppType app;
	bool debugMode;
	bool running;

public:
	Engine(int logW, int logH) : state(1600, 900, logW, logH)
	{
		debugMode = false;
		running = false;
		prevTime = 0;
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
		const float fixedStep = 1.0f / 60.0f;
		float accumulator = 0;
		prevTime = SDL_GetTicks();

		running = true;
		while (running)
		{
			// calculate deltaTime
			uint64_t nowTime = SDL_GetTicks();
			float deltaTime = (nowTime - prevTime) / 1000.0f;
			prevTime = nowTime;

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
						inputState.addEvent(event.key.scancode, false);
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

			Resources &res = Resources::getInstance();
			FrameContext ctx(state, inputState, fixedStep);

			World &world = World::getInstance();
			GameObject &root = world.getObject(app.getRoot());

			accumulator += deltaTime;
			if (accumulator >= fixedStep)
			{
				update(ComponentStage::Input, root, world, ctx);
				update(ComponentStage::Physics, root, world, ctx);
				update(ComponentStage::Gameplay, root, world, ctx);
				update(ComponentStage::Animation, root, world, ctx);

				accumulator -= fixedStep;
			}

			SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
			SDL_RenderClear(state.renderer);

			update(ComponentStage::Render, root, world, ctx);

			SDL_RenderPresent(state.renderer);

			update(ComponentStage::PostRender, root, world, ctx);

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
		}
	}

	void update(ComponentStage stage, GameObject &obj, World &world, const FrameContext &ctx)
	{
		obj.update(stage, ctx);

		auto &children = obj.getChildren();
		for (GHandle &hChild : children)
		{
			GameObject &child = world.getObject(hChild);
			update(stage, child, world, ctx);
		}
	}
};