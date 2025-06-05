#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace tmx
{
struct Layer;
struct ObjectGroup;
struct TileSet;

struct Map
{
	int mapWidth;
	int mapHeight;
	std::vector<TileSet> tileSets;
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

struct TileSet
{
	int count, tileWidth, tileHeight, rows, columns, firstgid;

public:
	TileSet(int firstgid, int count, int tileWidth, int tileHeight, int columns)
		: firstgid(firstgid), count(count), tileWidth(tileWidth), tileHeight(tileHeight), columns(columns)
	{
		rows = count / columns;
	}
};

}
