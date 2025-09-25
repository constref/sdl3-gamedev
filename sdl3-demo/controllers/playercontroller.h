#pragma once

#include "../controller.h"

class PlayerController : public Controller
{
public:
	PlayerController(GameObject &owner);
	void eventHandler(const FrameContext &ctx, int eventId) override;
	void collisionHandler(GameObject &other, glm::vec2 overlap) override;
};