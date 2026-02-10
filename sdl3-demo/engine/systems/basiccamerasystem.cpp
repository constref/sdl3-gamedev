#include <systems/basiccamerasystem.h>
#include <resources.h>
#include <systems/context/rendercontext.h>

BasicCameraSystem::BasicCameraSystem(Services &services) : System(services)
{
}

void BasicCameraSystem::update(Node &node)
{
	auto [ic, bc] = getRequiredComponents(node);

	glm::vec2 camPos{
		(node.getPosition().x + bc->getTileWidth() / 2) - bc->getViewportSize().x / 2,
		node.getPosition().y - bc->getViewportSize().y + 100
	};

	RenderContext::shared().setCameraPosition(camPos);
}
