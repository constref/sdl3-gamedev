#pragma once

class Component;

struct ComponentEntry
{
	size_t id;
	Component *comp;

	ComponentEntry(size_t id, Component *comp) : id(id), comp(comp) { }
};