#ifndef GAME_OBJECT_POOL
#define GAME_OBJECT_POOL
#include "PassInfo.hpp"
#include "GraphicsPipeline.h"
#include <vector>
#include <string.h>

class GameObjectPool
{
private:
    VulkanContext vkContext;
    

public:
    GameObjectPool();
    void init(VulkanContext context);
    void uploadVBOsAndIBOs();
    void drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame);
    void cleanUpResources();
    ~GameObjectPool();
};

#endif