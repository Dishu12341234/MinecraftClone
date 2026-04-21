#pragma once
#include <memory.h>
#include "Camera.h"

class Camera;


class Player
{
private:
    std::shared_ptr<Camera> camera;
    friend class HelloTriangleApplication;
    bool jumpTriggered = false;

    PlayerState playerState;
    uint healtPoints = 20;
    // int LBOC = 0;//last block when the player was in contact with the ground
    // int LZOC = 0;//last z when the player was not in contact with the ground

    int dz = 0;

    long long jumpStartTime;


    AxisAlignedBoundingBox aabb;
    GameObjectPool &gameObjectPool;

    //fornt -> NORTH ,left -> WEST
    CardinalFacing facingDirection{};

    Transform selfTransform;

    friend class HelloTriangleApplication;
public:
    float jump(float t)
    {
        return cos(t * 1.2f);
    }

    Player(VulkanContext &vkContext, GameObjectPool &gop);
    void drawUIIfPossible(DrawInfo &drawInfo, UI &ui);

    void handlePlayerMovement(UniformBufferObject &UBO,
                              VkExtent2D &swapChainExtent,
                              Event &event);


    const int getHealthPoints();

    ~Player();
};
