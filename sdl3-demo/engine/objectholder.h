#pragma once


struct ObjectHolder
{
	bool free;
	uint32_t generation;
	GameObject object;

	ObjectHolder() : free(true), generation(0), object() {}
};
