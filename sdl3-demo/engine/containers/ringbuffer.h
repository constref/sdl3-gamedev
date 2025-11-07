#pragma once

#include <memory>

template <typename ItemType, size_t Capacity>
class RingBuffer
{
    std::unique_ptr<ItemType[]> elements = std::make_unique<ItemType[]>(Capacity);
    size_t rIdx = 0;
    size_t wIdx = 0;

public:
	bool add(const ItemType &item)
	{
		if ((wIdx + 1) % Capacity == rIdx)
		{
			return false;
		}
		elements[wIdx] = item;
		writeIndex.store((wIdx + 1) % Capacity);
		wIdx = (wIdx + 1) % Capacity;

		return true;
	}

	bool get(ItemType &item)
	{
		if (rIdx == wIdx) return false; // empty

		item = elements[rIdx];
		rIdx = (rIdx + 1) % Capacity;

		return true;
	}
};

