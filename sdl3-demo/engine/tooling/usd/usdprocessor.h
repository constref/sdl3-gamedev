#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "rendering/mesh.h"

class Node;
class Services;

class USDProcessor
{
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;

public:
	void loadStage(const std::string &usdPath, Node& root, Services& services);
};
