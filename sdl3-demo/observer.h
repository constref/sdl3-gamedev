#pragma once

#include <vector>
#include <functional>

template <typename... Args>
class Subject
{
	using Observer = std::function<void(Args...)>;
	std::vector<Observer> observers;

public:
	void operator+=(const Observer &observer)
	{
		addObserver(observer);
	}
	void addObserver(const Observer &observer)
	{
		observers.push_back(observer);
	}

	void notify(Args... args)
	{
		for (const auto &observer : observers)
		{
			observer(args...);
		}
	}
};