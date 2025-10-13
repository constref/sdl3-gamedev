#include <SDL3/SDL_main.h>
#include <engine.h>

#include "platformer.h"

using namespace std;

int main(int argc, char *argv[])
{
	Engine<Platformer> engine(512, 288);
	if (!engine.initialize())
	{
		return 1;
	}

	engine.run();
	engine.cleanup();

	return 0;
}

/*
void drawParalaxBackground(const SDLState &state, const GameState &gs, SDL_Texture *texture,
	float xVelocity, float &scrollPos, float scrollFactor, float deltaTime)
{
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w)
	{
		scrollPos = 0;
	}

	SDL_FRect dst{
		.x = scrollPos, .y = static_cast<float>(state.logH - texture->h),
		.w = texture->w * 2.0f,
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(state.renderer, texture, nullptr, 1, &dst);
}
*/
