#pragma once

#include <vector>
#include <functional>

#include <unordered_map>
#include <string>
#include <cassert>

template<typename T>
class Subject
{
public:
	using Observer = std::function<void(const T&)>;

	void operator+=(const Observer &observer)
	{
		addObserver(observer);
	}
	void addObserver(const Observer &observer)
	{
		observers.push_back(observer);
	}

	void notify(const T& data)
	{
		for (const auto &observer : observers)
		{
			observer(data);
		}
	}

private:
	std::vector<Observer> observers;
};

class SubjectRegistry
{
	std::unordered_map<std::string, void *> subjects;

public:
	void registerSubject(const std::string &name, void *subject)
	{
		subjects[name] = subject;
	}

	template<typename T>
	void addObserver(const std::string &name, Subject<const T&>::Observer observer)
	{
		auto itr = subjects.find(name);
		assert(itr != subjects.end());

		Subject<const T&> *subject = static_cast<Subject<const T&> *>(itr->second);
		subject->addObserver(observer);
	}
};