#include "GameMeshObject.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "Textures.hpp"

// TODO: re-use IDs

GameMeshObject::GameMeshObject()
{
    
}

GameMeshObject::GameMeshObject(std::string modelPath)
{
    loadMeshModel(modelPath);
}

void GameMeshObject::loadMeshModel(std::string modelPath)
{
    vertices.clear();
    indices.clear();


    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err, warn;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
    {
        throw std::runtime_error(err);
    }
    if (warn.empty() == false)
    {
        std::cout << "WARN: " << warn << std::endl;
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void GameMeshObject::cleanUpResources()
{
    vkDestroyBuffer(vkContext.device, indexBuffer, nullptr);
    vkFreeMemory(vkContext.device, indexBufferMemory, nullptr);

    vkDestroyBuffer(vkContext.device, vertexBuffer, nullptr);
    vkFreeMemory(vkContext.device, vertexBufferMemory, nullptr);
}

GameMeshObject::~GameMeshObject()
{
    
}
