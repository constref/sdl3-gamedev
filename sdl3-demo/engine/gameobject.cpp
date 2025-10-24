#include "gameobject.h"
#include <world.h>

GameObject &GameObject::getObject(const GHandle &handle)
{
	World &world = World::get();
	return world.getObject(handle);
}
