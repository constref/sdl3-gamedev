#pragma once

#include <vector>
#include <node.h>
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

	NodeHandle createNode()
	{
		assert(!freeList.empty() && "Out of object slots in pool");

		size_t idx = freeList.back();
		freeList.pop_back();

		Holder &holder = objects[idx];
		holder.generation++;
		holder.free = false;
		holder.object.emplace(NodeHandle(idx, holder.generation));

		return holder.object.value().getHandle();
	}

	void free(NodeHandle handle)
	{
		assert(handle.index < objects.size() - 1 && "Object handle index out-of-bounds!");
		Holder &holder = objects[handle.index];
		if (handle.generation == holder.generation && !holder.free)
		{
			holder.free = true;
			holder.object.reset();

			assert(freeList.size() < MaxObjects && "Free list is already at capacity.");
			freeList.push_back(handle.index);
		}
	}

	Node &getNode(const NodeHandle handle)
	{
		Holder &holder = objects[handle.index];
		assert(!holder.free && "Attempted to access a freed object");
		assert(holder.generation == handle.generation && "Attempted to access an invalid object handle");
		return holder.object.value();
	}

	size_t getFreeCount() const
	{
		return freeList.size();
	}
};

class World : public ObjectPool<Node, 5000>
{
	World() { }

public:
	static World &get()
	{
		static World instance;
		return instance;
	}
};