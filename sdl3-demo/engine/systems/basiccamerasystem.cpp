#include <systems/basiccamerasystem.h>
#include <resources.h>
#include <systems/context/rendercontext.h>

BasicCameraSystem::BasicCameraSystem(Services &services, glm::vec2 viewportSize, int tileWidth, int tileHeight, int mapWidth, int mapHeight) : System(services)
{
	this->viewportPosition = { 0, 0 };
	this->viewportSize = viewportSize;
	this->tileWidth = tileWidth;
	this->mapWidth = mapWidth;
	this->mapHeight = mapHeight;
}

void BasicCameraSystem::update(Node &node)
{
	auto [ic, bc] = getRequiredComponents(node);

	glm::vec2 camPos {
		(node.getPosition().x + tileWidth / 2) - viewportSize.x / 2,
		mapHeight * tileHeight - viewportSize.y + 640
	};

	RenderContext::shared().setCameraPosition(camPos);
}
