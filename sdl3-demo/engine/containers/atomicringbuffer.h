#pragma once

#include <atomic>
#include <memory>

template <typename ItemType, size_t Capacity>
class RingBuffer
{
    std::unique_ptr<ItemType[]> elements = std::make_unique<ItemType[]>(Capacity);
    std::atomic_size_t readIndex = 0;
    std::atomic_size_t writeIndex = 0;

public:
	bool add(const ItemType &item)
	{
		const size_t rIdx = readIndex.load(std::memory_order_relaxed);
		const size_t wIdx = writeIndex.load(std::memory_order_relaxed);
		if ((wIdx + 1) % Capacity == rIdx)
		{
			return false;
		}
		elements[wIdx] = item;
		writeIndex.store((wIdx + 1) % Capacity);

		return true;
	}

	bool get(ItemType &item)
	{
		const size_t rIdx = readIndex.load(std::memory_order_relaxed);
		const size_t wIdx = writeIndex.load(std::memory_order_acquire);
		if (rIdx == wIdx) return false; // empty

		item = elements[rIdx];
		readIndex.store((rIdx + 1) % Capacity, std::memory_order_release);

		return true;
	}
};

