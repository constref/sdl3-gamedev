#pragma once

#include <glm/glm.hpp>

class RenderContext
{
	glm::vec2 camPosition;

public:
	static RenderContext &shared()
	{
		static RenderContext ctx;
		return ctx;
	}

	glm::vec2 getCameraPosition() const { return camPosition; }
	void setCameraPosition(const glm::vec2 &position) { camPosition = position; }
};