#pragma once

#include <systems/system.h>
#include "../events.h"
#include "nodeauthoringcomponent.h"

struct SDL_Window;

class EditorSystem : public System<FrameStage::End, NodeAuthoringComponent>
{
    SDL_Window *m_window;
public:
    EditorSystem(Services& services);
    void onEvent(const MouseRelativeModeToggleEvent &event) const;
    void update(Node& node) override;
};
