#include "project.h"

std::ofstream persistence::createFile(const std::string &filepath)
{
    std::ofstream file(filepath, std::ios::binary);
    return file;
}

void persistence::finish(std::ofstream &file) { file.close(); }
void persistence::writeMeshFile(const std::string &assetId, const Mesh &mesh) {}
