#pragma once

#include <components/component.h>

namespace usd
{
class RootComponent : public Component
{
public:
    explicit RootComponent(Node& owner) : Component(owner)
    {
    }
};
}
