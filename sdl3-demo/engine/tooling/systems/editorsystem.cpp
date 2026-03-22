#include "editorsystem.h"

#include <SDL3/SDL.h>

EditorSystem::EditorSystem(Services& services) : System(services)
{
}

void EditorSystem::onEvent(const MouseRelativeModeToggleEvent& event) const
{
    Logger::info(this, "Relative mouse mode toggled");
}

void EditorSystem::update(Node& node)
{
}
