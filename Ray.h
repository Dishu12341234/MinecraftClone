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

    RayMesher rayMesher;
public:
    Ray(VulkanContext &vkContext, GameObjectPool &gameObjectPool);
    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
                 std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void cleanup();
    ~Ray();
};
