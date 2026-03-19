#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <systems/system.h>
#include <components/meshcomponent.h>
#include <rendering/mesh.h>

class Node;
class Services;

struct PrimGeo
{
	std::unique_ptr<Mesh> mesh;
	GPUMeshHandle gpuHandle;
};

namespace usd
{
class USDSystem
{
	std::unordered_map<std::string, PrimGeo> meshes;

public:
	USDSystem();
	void createStage(const std::string& path);
	void saveStage();
	void addLayer(const std::string& layerPath);
	void loadStage(const std::string &usdPath, Node& root, Services& services);
};
}

