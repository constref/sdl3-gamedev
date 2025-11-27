#include <SDL3/SDL_main.h>
#include <engine.h>

#include "platformer.h"

using namespace std;

int main(int argc, char *argv[])
{
	Engine<Platformer> engine;
	if (!engine.initialize(512, 288))
	{
		return 1;
	}

	engine.run();
	engine.cleanup();

	return 0;
}
