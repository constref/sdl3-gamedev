#pragma once

#include <systems/system.h>

#include "components/inputcomponent.h"

class MouseRelativeModeToggleEvent;
struct SDL_Window;

class WindowingSystem : public System<FrameStage::Start, InputComponent>
{
    int m_width;
    int m_height;
    SDL_Window *m_window;
    bool m_relativeMouseMode;
public:
    WindowingSystem(Services& services);
    virtual ~WindowingSystem();
    
    void onEvent(NodeHandle target, const MouseRelativeModeToggleEvent &event);
    
    bool initialize(int width, int height);
    void shutdown() const;
    SDL_Window *window() const;
    void update(Node& node) override;
};
