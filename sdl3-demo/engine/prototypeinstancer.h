#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <nodehandle.h>
#include <prototypeid.h>

class Node;
class Services;

using FactoryFunc = std::function<Node*(NodeHandle)>;

class PrototypeInstancer
{
	std::unordered_map<PrototypeId, FactoryFunc> factories;

public:
	PrototypeId registerFactory(const std::string &name, FactoryFunc func)
	{
		std::hash<std::string> hasher;
		PrototypeId protoId = hasher(name);
		factories[protoId] = func;
		return protoId;
	}

	Node *instanciate(PrototypeId protoId, NodeHandle hParent)
	{
		return factories[protoId](hParent);
	}
};

