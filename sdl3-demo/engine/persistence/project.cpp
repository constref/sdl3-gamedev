#include "project.h"
#include <fstream>
#include <ios>
#include <memory>
#include <optional>
#include <rendering/mesh.h>
#include <rendering/vertex.h>
#include <vector>

#include "uuid.h"

std::ofstream persistence::createFile(const std::string &filepath)
{
    std::ofstream file(filepath, std::ios::binary);
    return file;
}

void persistence::finish(std::ofstream &file) { file.close(); }

std::unique_ptr<Mesh> persistence::readMeeshFile(const std::string &assetId)
{
    std::string assetPath = "baked/assets/" + assetId + ".nmo";
    std::ifstream file(assetPath, std::ios::binary);

    Mesh *mesh = new Mesh;
    MeshHeader meshHeader;
    file.read(reinterpret_cast<char *>(&meshHeader), sizeof(MeshHeader));

    for (int i = 0; i < meshHeader.subMeshCount; ++i)
    {
        SubMeshHeader smHeader;
        file.read(reinterpret_cast<char *>(&smHeader), sizeof(SubMeshHeader));

        // read vertex data
        SubMesh subMesh;
        subMesh.vertices.resize(smHeader.vertexCount);
        subMesh.indices.resize(smHeader.indexCount);
        file.read(reinterpret_cast<char *>(subMesh.vertices.data()), subMesh.vertexByteSize());
        file.read(reinterpret_cast<char *>(subMesh.indices.data()), subMesh.indexByteSize());
        mesh->addSubmesh(std::move(subMesh));
    }
    file.close();

    return std::unique_ptr<Mesh>(mesh);
}

void persistence::writeMeshFile(const std::string &assetId, const Mesh &mesh)
{
    std::string outputPath = "baked/assets/" + assetId + ".nmo";
    std::ofstream file(outputPath, std::ios::binary);

    std::optional<uuids::uuid> assetUuid = uuids::uuid::from_string(assetId);
    assert(assetUuid.has_value() && "Invalid asset-id UUID provided");

    MeshHeader meshHeader;
    meshHeader.id = assetUuid.value();
    meshHeader.subMeshCount = mesh.subMeshes().size();
    file.write(reinterpret_cast<char *>(&meshHeader), sizeof(MeshHeader));

    for (const SubMesh &sm : mesh.subMeshes())
    {
        SubMeshHeader subMeshHeader;
        subMeshHeader.vertexCount = sm.vertices.size();
        subMeshHeader.indexCount = sm.indices.size();
        file.write(reinterpret_cast<const char *>(&subMeshHeader), sizeof(SubMeshHeader));

        // write out vertices
        file.write(reinterpret_cast<const char *>(sm.vertices.data()), sm.vertexByteSize());
        // write out indices
        file.write(reinterpret_cast<const char *>(sm.indices.data()), sm.indexByteSize());
    }
    file.close();
}
