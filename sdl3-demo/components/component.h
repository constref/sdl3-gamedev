#pragma once

struct GameObject;
struct FrameContext;

class Component
{
public:
	Component() {}
	virtual ~Component() {}
	virtual void update(GameObject &owner, const FrameContext &ctx) = 0;
};
