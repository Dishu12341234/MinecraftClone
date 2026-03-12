#ifndef GAME_MESH_OBJECT_H
#define GAME_MESH_OBJECT_H

#include "include/tinyobjloader/tiny_obj_loader.h"
#include "GraphicsPipeline.h"
#include <vector>
#include <string>
#include <iostream>
#include <atomic>

class GameMeshObject
{
private:

    VulkanContext vkContext;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    //TODO impliment VBO
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    

    friend class MeshUploader;
    friend class GameObject;
    friend class HelloTriangleApplication;
public:
    GameMeshObject();
    GameMeshObject(std::string modelPath);
    void loadMeshModel(std::string modelPath);
    void cleanUpResources();
    ~GameMeshObject();
};

#endif