#include <systems/basiccamerasystem.h>
#include <resources.h>
#include <systems/context/rendercontext.h>

BasicCameraSystem::BasicCameraSystem(glm::vec2 viewportSize)
{
	this->viewportPosition = { 0, 0 };
	this->viewportSize = viewportSize;
}

void BasicCameraSystem::update(Node &node)
{
	auto [ic, bc] = getRequiredComponents(node);
	const Resources &res = Resources::get();

	glm::vec2 camPos {
		(node.getPosition().x + res.map->tileWidth / 2) - viewportSize.x / 2,
		res.map->mapHeight * res.map->tileHeight - viewportSize.y
	};

	RenderContext::shared().setCameraPosition(camPos);
}
