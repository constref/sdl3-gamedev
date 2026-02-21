#include <SDL3/SDL_main.h>
#include <bootstrap.h>
#include "platformer.h"

using namespace std;

int main(int argc, char *argv[])
{
	Bootstrap bootstrap(argc, argv);
	bootstrap.exec(std::make_unique<Platformer>(), 512, 288, 1920, 1080);

	return 0;
}
