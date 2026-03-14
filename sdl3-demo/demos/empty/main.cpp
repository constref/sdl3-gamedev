#include <SDL3/SDL_main.h>
#include <bootstrap.h>
#include "empty.h"

using namespace std;

int main(int argc, char *argv[])
{
	Bootstrap bootstrap(argc, argv);
	bootstrap.exec(std::make_unique<Empty>(), 1280, 700, 1280, 700);

	return 0;
}
