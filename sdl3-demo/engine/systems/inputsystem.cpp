#include <systems/inputsystem.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <messaging/commands.h>
#include <inputstate.h>
#include <world.h>

InputSystem::InputSystem()
{
	direction = { 0, 0 };
	EventQueue::get().dispatcher.registerHandler2<KeyboardEvent>(this);
}

void InputSystem::update(Node &node)
{
}

void InputSystem::onEvent(NodeHandle hNode, const KeyboardEvent &event)
{
	Node &node = World::get().getNode(hNode);
	auto [ic] = getRequiredComponents(node);

	auto &state = InputState::get();
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
	if (this->direction != direction)
	{
		this->direction = direction;
		EventQueue::get().enqueue2<DirectionChanged>(hNode, 0, direction);
	}

	Node &owner = World::get().getNode(hNode);
	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue2<JumpEvent>(owner.getHandle(), 0);
			}
			break;
		}
		case SDL_SCANCODE_J:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue2<ShootBeginEvent>(owner.getHandle(), 0);
			}
			else
			{
				EventQueue::get().enqueue2<ShootEndEvent>(owner.getHandle(), 0);
			}
			break;
		}
	}
}
