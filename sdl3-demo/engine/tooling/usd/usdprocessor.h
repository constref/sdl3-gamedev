#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "components/meshcomponent.h"
#include "rendering/mesh.h"

class Node;
class Services;

struct PrimGeo
{
	std::unique_ptr<Mesh> mesh;
	GPUMeshHandle gpuHandle;
};

class USDProcessor
{
	std::unordered_map<std::string, PrimGeo> meshes;

public:
	void loadStage(const std::string &usdPath, Node& root, Services& services);
};
