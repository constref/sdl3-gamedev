#pragma once

struct FrameContext;
class GameObject;
class SubjectRegistry;
class MessageDispatch;

enum class ComponentStage
{
	Input = 0,
	Physics = 1,
	Gameplay = 2,
	Animation = 3,
	Render = 4,
	PostRender = 5,
	SIZE
};

class Component
{
	ComponentStage stage;

protected:
	GameObject &owner;

public:
	Component(GameObject &owner, ComponentStage stage) : owner(owner), stage(stage) {}
	ComponentStage getStage() const { return stage; }
	virtual ~Component() {}
	virtual void onAttached(MessageDispatch &dispatch) {}
	virtual void onStart() {}
	virtual void update(const FrameContext &ctx) = 0;
	void emit(const FrameContext &ctx, int eventId);
};
