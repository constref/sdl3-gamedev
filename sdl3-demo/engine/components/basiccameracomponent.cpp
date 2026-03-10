#include "basiccameracomponent.h"

#include <node.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/commands.h>
#include <messaging/commanddispatcher.h>
#include <world.h>

BasicCameraComponent::BasicCameraComponent(Node &owner, glm::vec2 viewportSize, int tileWidth, int tileHeight, int mapWidth, int mapHeight) : Component(owner)
{
	this->viewportPosition = { 0, 0 };
	this->viewportSize = viewportSize;
	this->tileWidth = tileWidth;
	this->mapWidth = mapWidth;
	this->mapHeight = mapHeight;
	position = { 0, 0 };
}
