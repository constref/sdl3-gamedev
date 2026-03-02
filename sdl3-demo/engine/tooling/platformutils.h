#pragma once

#include <string>

struct SDL_Window;

namespace PlatformUtils
{

void showMessage(const std::string &title, const std::string &message, SDL_Window *window = nullptr);

}
