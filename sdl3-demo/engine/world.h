#pragma once

#include <vector>
#include <gameobject.h>
#include <objectholder.h>

template<typename T, size_t MaxObjects>
class ObjectPool
{
	using Holder = ObjectHolder<T>;

	std::vector<Holder> objects;
	std::vector<size_t> freeList;

public:
	ObjectPool()
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

	GHandle createObject()
	{
		assert(!freeList.empty() && "Out of object slots in pool");

		size_t idx = freeList.back();
		freeList.pop_back();

		Holder &holder = objects[idx];
		holder.generation++;
		holder.free = false;

		return GHandle(idx, holder.generation);
	}

	void freeObject(GHandle handle)
	{
		assert(handle.index < objects.size() - 1 && "Object handle index out-of-bounds!");
		Holder &holder = objects[handle.index];
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
		Holder &holder = objects[handle.index];
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

class World : public ObjectPool<GameObject, 2000>
{
	World() { }

public:
	static World &getInstance()
	{
		static World instance;
		return instance;
	}
};