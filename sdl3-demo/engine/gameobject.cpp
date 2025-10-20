#include "gameobject.h"
#include <world.h>

GameObject &GameObject::getObject(const GHandle &handle)
{
	World &world = World::getInstance();
	return world.getObject(handle);
}
