#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace tmx
{

struct Layer
{
	int id;
	std::string name;
	std::vector<int> data;
};

struct LayerObject
{
	int id;
	std::string name, type;
	float x, y;
};

struct ObjectGroup
{
	int id;
	std::string name;
	std::vector<LayerObject> objects;
};

struct Image
{
	std::string source;
	int width, height;
};

struct Tile
{
	int id;
	Image image;
};

struct TileSet
{
	int count, tileWidth, tileHeight, rows, columns, firstgid;
	std::vector<Tile> tiles;

public:
	TileSet(int firstgid, int count, int tileWidth, int tileHeight, int columns)
		: firstgid(firstgid), count(count), tileWidth(tileWidth), tileHeight(tileHeight), columns(columns)
	{
		rows = columns ? count / columns : 0;
	}
};

struct Map
{
	int mapWidth, mapHeight;
	int tileWidth, tileHeight;
	std::vector<TileSet> tileSets;
	std::vector<std::variant<Layer, ObjectGroup>> layers;
};

std::unique_ptr<Map> loadMap(const std::string &mapFilePath);

}
