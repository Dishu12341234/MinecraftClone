#include "HelloTriangleApplication.hpp"
#include "Camera.h"
#include "Chunk.h"
#include <cstdlib>
#include <cstring>
#include <memory>
#include "Ray.h"
#include "iostream"

void HelloTriangleApplication::initGameObjects()
{
    static VulkanContext context{};
    context.device = device;
    context.physicalDevice = physicalDevice;
    context.graphicsQueue = graphicsQueue;
    context.instance = instance;
    context.presentQueue = presentQueue;
    context.commandPool = commandPool;

    gameObjectPool.uploadVBOsAndIBOs();

    camera = std::make_unique<Camera>(context, gameObjectPool);

    terrain = std::move(Terrain(context, gameObjectPool));
    gameObjectPool.terrain = &terrain.value();
    terrain.value().generateChunks();
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage)
{

    std::cout << "Coordinates: " << "(x,y,z) (x" << camera->gePositionInWorldCoords().x << ", " << camera->gePositionInWorldCoords().y << ", " << camera->gePositionInWorldCoords().z << ")" << std::endl;

    HitInfo hitInfo{};
    camera->getHitInfo(hitInfo);

    if(hitInfo.hitVoxel)
    {
        std::cout << "Hit block type: " << hitInfo.hitVoxel->getBlockType() << std::endl;
        std::cout << "Hit block coordinates: (x,y,z) (" << hitInfo.blockCoords.x << ", " << hitInfo.blockCoords.y << ", " << hitInfo.blockCoords.z << ")" << std::endl;
    }

    if (event->getKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(_window, GLFW_TRUE);

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};

    // Model 0
    // ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.model = glm::translate(glm::mat4(1.f), glm::vec3(0.f));
    ubo.model = glm::rotate(ubo.model, 3 * time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.model = glm::rotate(ubo.model, 3 * time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));

    camera->updateUBO(ubo, swapChainExtent, *event.get());

    // ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 1.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    // ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.01f, 100.0f);

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}