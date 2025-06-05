#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace tmx
{
struct Layer;
struct ObjectGroup;

struct Map
{
	int mapWidth;
	int mapHeight;
	std::vector<std::variant<Layer, ObjectGroup>> layers;
};

struct LayerObject
{
	int id;
	std::string name, type;
	float x, y;
};

struct Layer
{
	int id;
	std::string name;
	std::vector<int> data;
};

struct ObjectGroup
{
	int id;
	std::string name;
	std::vector<LayerObject> objects;
};

}
