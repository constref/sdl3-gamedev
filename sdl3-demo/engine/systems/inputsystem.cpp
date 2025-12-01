#include <systems/inputsystem.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <messaging/commands.h>
#include <inputstate.h>
#include <world.h>

InputSystem::InputSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<KeyUpEvent>(this);
	services.eventQueue().dispatcher.registerHandler<KeyDownEvent>(this);
}

void InputSystem::update(Node &node)
{
}

void InputSystem::handleDirectionChange(NodeHandle target, InputState &state, InputComponent &inputComp)
{
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
	if (inputComp.getDirection() != direction)
	{
		inputComp.setDirection(direction);
		services.eventQueue().enqueue<DirectionChangedEvent>(target, 0, direction);
	}
}

void InputSystem::onEvent(NodeHandle target, const KeyDownEvent &event)
{
	Node &node = services.world().getNode(target);
	auto [ic] = getRequiredComponents(node);

	auto &state = services.inputState();
	state.setKeyState(event.scancode, true);
	handleDirectionChange(target, state, *ic);

	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			services.eventQueue().enqueue<JumpEvent>(node.getHandle(), 0);
			break;
		}
		case SDL_SCANCODE_J:
		{
			services.eventQueue().enqueue<ShootBeginEvent>(node.getHandle(), 0);
			break;
		}
	}
}

void InputSystem::onEvent(NodeHandle target, const KeyUpEvent &event)
{
	Node &node = services.world().getNode(target);
	auto [ic] = getRequiredComponents(node);

	auto &state = services.inputState();
	state.setKeyState(event.scancode, false);
	handleDirectionChange(target, state, *ic);

	switch (event.scancode)
	{
		case SDL_SCANCODE_J:
		{
			services.eventQueue().enqueue<ShootEndEvent>(node.getHandle(), 0);
		}
	}
}

void InputSystem::onLinked(Node &node)
{
	services.inputState().setFocus(node.getHandle());
}
