#pragma once

#include <memory>
#include <vector>

class CollisionComponent;

class CollisionSystem
{
	std::vector <std::shared_ptr<CollisionComponent>> components;

public:
	void add(std::shared_ptr<CollisionComponent> component);
};