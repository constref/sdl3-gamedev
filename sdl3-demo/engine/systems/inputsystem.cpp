#include <systems/inputsystem.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <messaging/commands.h>
#include <inputstate.h>
#include <world.h>

InputSystem::InputSystem()
{
	EventQueue::get().dispatcher.registerHandler2<KeyboardEvent>(this);
}

void InputSystem::update(Node &node)
{
}

void InputSystem::onEvent(NodeHandle hNode, const KeyboardEvent &event)
{
	auto &state = InputState::get();
	state.setKeyState(event.scancode, event.state == KeyboardEvent::State::down);

	float direction = 0;
	if (state.isKeyPressed(SDL_SCANCODE_A))
	{
		direction += -1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_D))
	{
		direction += 1;
	}

	Node &owner = World::get().getNode(hNode);
	owner.sendCommand(UpdateDirectionCommand{ direction });

	switch (event.scancode)
	{
		case SDL_SCANCODE_K:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue<JumpEvent>(owner.getHandle(), 0);
			}
			break;
		}
		case SDL_SCANCODE_J:
		{
			if (event.state == KeyboardEvent::State::down)
			{
				EventQueue::get().enqueue<ShootBeginEvent>(owner.getHandle(), 0);
			}
			else
			{
				EventQueue::get().enqueue<ShootEndEvent>(owner.getHandle(), 0);
			}
			break;
		}
	}
}
