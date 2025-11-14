#include <systems/inputsystem.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <messaging/commands.h>
#include <inputstate.h>
#include <world.h>

InputSystem::InputSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler2<KeyboardEvent>(this);
}

void InputSystem::update(Node &node)
{
}

void InputSystem::onEvent(NodeHandle hNode, const KeyboardEvent &event)
{
	Node &node = services.world().getNode(hNode);
	auto [ic] = getRequiredComponents(node);

	auto &state = services.inputState();
	state.setKeyState(event.scancode, event.state == KeyboardEvent::State::down);

	glm::vec2 direction{ 0 };
	if (state.isKeyPressed(SDL_SCANCODE_A))
	{
		direction.x += -1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_D))
	{
		direction.x += 1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_W))
	{
		direction.y += 1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_S))
	{
		direction.y -= 1;
	}
	if (ic->getDirection() != direction)
	{
		ic->setDirection(direction);
		services.eventQueue().enqueue<DirectionChangedEvent>(hNode, 0, direction);
	}

	Node &owner = services.world().getNode(hNode);
	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				services.eventQueue().enqueue<JumpEvent>(owner.getHandle(), 0);
			}
			break;
		}
		case SDL_SCANCODE_J:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				services.eventQueue().enqueue<ShootBeginEvent>(owner.getHandle(), 0);
			}
			else
			{
				services.eventQueue().enqueue<ShootEndEvent>(owner.getHandle(), 0);
			}
			break;
		}
	}
}
