#include "meshloader.h"

#include <DirectXMath.h>
#include <memory>
#include <rendering/mesh.h>
#include <tiny_gltf.h>

namespace assets
{
LoadResult loadGltf(const std::string &path)
{
    tinygltf::Model model;

    tinygltf::LoadImageDataFunction imageLoaderFunc =
        [](tinygltf::Image *image, const int imgIndex, std::string *err, std::string *warn, int reqWidth, int reqHeight,
           const unsigned char *bytes, int size, void *userData) -> bool { return false; };

    tinygltf::TinyGLTF loader;
    loader.SetImageLoader(imageLoaderFunc, nullptr);

    std::string err;
    std::string warn;
    loader.LoadASCIIFromFile(&model, &err, &warn, path);

    LoadResult result;

    // load images
    for (const tinygltf::Image &image : model.images)
    {
        // auto [imageId, img] = createImage(image.width, image.height, image.component);
        // if (!imageId.isValid())
        // {
        //     showError("Image could not be loaded");
        //     break;
        // }
    }

    // load all meshes first
    for (const tinygltf::Mesh &mesh : model.meshes)
    {
        Mesh *newMesh = new Mesh();
        for (const tinygltf::Primitive &primitive : mesh.primitives)
        {
            SubMesh subMesh;
            // load primitive vertices into sub-mesh
            if (const auto &itr = primitive.attributes.find("POSITION"); itr != primitive.attributes.end())
            {
                const auto &[name, index] = *itr;
                const tinygltf::Accessor &access = model.accessors[index];
                const tinygltf::BufferView &bv = model.bufferViews[access.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bv.buffer];

                if (access.type == TINYGLTF_TYPE_VEC3)
                {
                    subMesh.vertices.reserve(access.count);
                    for (int i = 0; i < access.count; ++i)
                    {
                        size_t offset = bv.byteOffset + access.byteOffset +
                                        i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(DirectX::XMFLOAT4));
                        const DirectX::XMFLOAT3 *pos =
                            reinterpret_cast<const DirectX::XMFLOAT3 *>(buffer.data.data() + offset);
                        subMesh.vertices.push_back(Vertex{.position = *pos});
                    }
                }
            }
            // load primitive vertices into sub-mesh
            if (const auto &itr = primitive.attributes.find("TEXCOORD_0"); itr != primitive.attributes.end())
            {
                const auto &[name, index] = *itr;
                const tinygltf::Accessor &access = model.accessors[index];
                const tinygltf::BufferView &bv = model.bufferViews[access.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bv.buffer];

                if (access.type == TINYGLTF_TYPE_VEC2)
                {
                    for (int i = 0; i < access.count; ++i)
                    {
                        size_t offset = bv.byteOffset + access.byteOffset +
                                        i * ((bv.byteStride > 0) ? bv.byteStride : sizeof(DirectX::XMFLOAT2));
                        const DirectX::XMFLOAT2 *uv =
                            reinterpret_cast<const DirectX::XMFLOAT2 *>(buffer.data.data() + offset);
                        subMesh.vertices[i].uv = *uv;
                    }
                }
            }
            // indices
            if (primitive.indices != -1)
            {
                const tinygltf::Accessor &access = model.accessors[primitive.indices];
                const tinygltf::BufferView &bv = model.bufferViews[access.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bv.buffer];

                if (access.type == TINYGLTF_TYPE_SCALAR && access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                {
                    subMesh.indices.reserve(access.count);
                    for (int i = 0; i < access.count; ++i)
                    {
                        const uint32_t *idx =
                            reinterpret_cast<const uint32_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) +
                            i;
                        subMesh.indices.push_back(*idx);
                    }
                }
                else if (access.type == TINYGLTF_TYPE_SCALAR &&
                         access.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    for (int i = 0; i < access.count; ++i)
                    {
                        const uint16_t *idx =
                            reinterpret_cast<const uint16_t *>(buffer.data.data() + bv.byteOffset + access.byteOffset) +
                            i;
                        subMesh.indices.push_back(*idx);
                    }
                }
            }
            newMesh->addSubmesh(std::move(subMesh));
        }
        result.meshes.insert({mesh.name, std::unique_ptr<Mesh>(newMesh)});
    }
    return result;
}
}; // namespace asssts
