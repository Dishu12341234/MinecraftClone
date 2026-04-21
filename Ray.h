#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "RayMesher.h"

class GameObjectPool;

class Ray
{
private:
    VulkanContext &vkContext;
    GameObjectPool &gameObjectPool;
    Transform transform{};
    RayMesher rayMesher;
    glm::vec3 direction = glm::vec3(1.f, 0.f, 0.f);



    friend class Camera;
public:
    Ray(VulkanContext &vkContext, GameObjectPool &gameObjectPool);
    void draw(DrawInfo &drawInfo);

    void cleanup();
    ~Ray();
};
