#pragma once

#include <vector>
#include <gameobject.h>

struct GHolder
{
	bool free;
	uint32_t generation;
	GameObject object;

	GHolder() : free(true), generation(0), object() {}
};

class World
{
	std::vector<GHolder> objects;
	std::vector<size_t> freeList;

	constexpr static int MaxObjects = 2000;
	World()
	{
		// preallocate objects
		objects.resize(MaxObjects);

		// generate freelist
		freeList.reserve(MaxObjects);
		for (size_t i = 0; i < objects.size(); ++i)
		{
			freeList.push_back(i);
		}
	}

public:
	static World &getInstance()
	{
		static World instance;
		return instance;
	}

	GHandle createObject()
	{
		assert(!freeList.empty() && "Out of object slots in World");

		size_t idx = freeList.back();
		freeList.pop_back();

		GHolder &holder = objects[idx];
		holder.generation++;
		holder.free = false;

		return GHandle(idx, holder.generation);
	}

	void freeObject(GHandle handle)
	{
		assert(handle.index < objects.size() - 1 && "Object handle index out-of-bounds!");
		GHolder &holder = objects[handle.index];
		if (handle.generation == 0 || holder.free ||
			holder.generation != handle.generation)
		{
			holder.free = true;
			assert(freeList.size() < MaxObjects && "Free list is already at capacity.");
			freeList.push_back(handle.index);
		}
	}

	GameObject &getObject(const GHandle handle)
	{
		GHolder &holder = objects[handle.index];
		assert(!holder.free && "Attempted to access a freed object");
		assert(holder.generation == handle.generation && "Attempted to access an invalid object handle");
		return holder.object;
	}

	auto &getObjects()
	{
		return objects;
	}

	size_t getFreeCount() const
	{
		return freeList.size();
	}
};