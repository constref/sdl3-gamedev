#include <systems/inputsystem.h>
#include <SDL3/SDL.h>
#include <messaging/events.h>
#include <messaging/eventqueue.h>
#include <messaging/commands.h>
#include <inputstate.h>
#include <world.h>

InputSystem::InputSystem(Services &services) : System(services)
{
	services.eventQueue().dispatcher.registerHandler<KeyUpEvent>(this);
	services.eventQueue().dispatcher.registerHandler<KeyDownEvent>(this);
	services.eventQueue().dispatcher.registerHandler<MouseMotionEvent>(this);
}

void InputSystem::update(Node &node)
{
}

void InputSystem::handleDirectionChange(NodeHandle target, const InputState &state, InputComponent &inputComp) const
{
	DirectX::XMINT3 axes = inputComp.getAxes();
	glm::vec3 direction{ 0 };
	if (state.isKeyPressed(SDL_SCANCODE_A))
	{
		direction[axes.x] += -1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_D))
	{
		direction[axes.x] += 1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_W))
	{
		direction[axes.y] += 1;
	}
	if (state.isKeyPressed(SDL_SCANCODE_S))
	{
		direction[axes.y] -= 1;
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

void InputSystem::onEvent(NodeHandle hNode, const MouseMotionEvent& event)
{
	using namespace DirectX;
	Node &node = services.world().getNode(hNode);
	auto [ic] = getRequiredComponents(node);
	XMFLOAT2 newPos((float)event.x(), (float)event.y());
	XMFLOAT2 oldPos = ic->mousePosition();
	XMVECTOR v1 = XMLoadFloat2(&newPos);
	XMVECTOR v2 = XMLoadFloat2(&oldPos);
	XMVECTOR delta = XMVectorSubtract(v1, v2);
	XMFLOAT2 fDelta;
	XMStoreFloat2(&fDelta, delta);
	ic->setMousePosition(newPos);
	ic->setMouseDelta(fDelta);
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
