#ifndef CAMERA_H
#define CAMERA_H

#include "GraphicsPipeline.h"
#include "Event.h"
#include "GameObjectPool.h"

class Camera
{
private:
    VulkanContext vkContext;
    glm::vec3 cameraPos{2.f, 2.f, 1.f};
    float pitch;
    float yaw;
    GameObjectPool &gameObjectPool;
    glm::vec3 forward;
    glm::vec3 worldUp{0, 0, 1};

public:
    Camera(VulkanContext vkContext, GameObjectPool &gop);
    void updateUBO(UniformBufferObject &UBO, VkExtent2D &swapChainExtent, Event &event);
    glm::vec3 gePositionInWorldCoords();

    ~Camera();
};

#endif