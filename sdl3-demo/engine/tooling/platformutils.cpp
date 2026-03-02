#include "platformutils.h"
#include <SDL3/SDL.h>

void PlatformUtils::showMessage(const std::string& title, const std::string& message, SDL_Window* window)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), window);
}
