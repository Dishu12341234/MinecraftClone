#pragma once
#include <optional>
#include "Camera.h"

class Camera;


class Player
{
private:
    std::optional<Camera> camera;
    friend class HelloTriangleApplication;
    bool jumpTriggered = false;

    PlayerState playerState;
    uint healtPoints = 20; 
    int LBOC = 0;//last block when the player was in contact with the ground
    int LZOC = 0;//last z when the player was not in contact with the ground

    int dz = 0;

    long long jumpStartTime;


    AxisAlignedBoundingBox aabb;
    GameObjectPool &gameObjectPool;

    friend class HelloTriangleApplication;
    void resolveAxis(float amount, int axis, float &vertVel);
    bool checkAABBOverlap(const AxisAlignedBoundingBox &a, const AxisAlignedBoundingBox &b);

public:
    float jump(float t)
    {
        return cos(t * 1.2f);
    }

    Player(VulkanContext &vkContext, GameObjectPool &gop);
    void drawUIIfPossible(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                          VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets,
                          uint32_t currentFrame, VkExtent2D &swapChainExtent, UI &ui);

    void handlePlayerMovement(UniformBufferObject &UBO,
                              VkExtent2D &swapChainExtent,
                              Event &event);


    const int getHealthPoints();

    ~Player();
};
