#include "external.h"

#include "platformer.h"
#include <engine.h>

int StartGame()
{
	Engine<Platformer> engine;
	if (!engine.initialize(512, 288))
	{
		return 1;
	}
	engine.run();

	return 0;
}
