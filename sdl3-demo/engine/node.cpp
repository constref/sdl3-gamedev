#include "node.h"
#include <world.h>

Node &Node::getObject(const NodeHandle &handle)
{
	World &world = World::get();
	return world.getObject(handle);
}
