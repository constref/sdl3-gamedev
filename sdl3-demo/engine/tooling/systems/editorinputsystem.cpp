#include "editorinputsystem.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include "inputstate.h"
#include "tooling/events.h"

EditorInputSystem::EditorInputSystem(Services& services) : System(services)
{
    services.eventQueue().dispatcher.registerHandler<KeyUpEvent>(this);
}

void EditorInputSystem::onEvent(NodeHandle target, const KeyUpEvent &event) const
{
    switch (event.scancode)
    {
        case SDL_SCANCODE_GRAVE:
        {
            services.eventQueue().enqueue<MouseRelativeModeToggleEvent>(services.inputState().getFocusTarget(), 0);
            break;
        }
    }
}

void EditorInputSystem::update(Node& node)
{
}
