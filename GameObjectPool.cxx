#include "GameObjectPool.h"

GameObjectPool::GameObjectPool()
{
}

void GameObjectPool::init(VulkanContext context)
{
    this->vkContext = context;  
}

void GameObjectPool::uploadVBOsAndIBOs()
{
    
}

void GameObjectPool::drawIndexed(VkCommandBuffer &commandBuffer, std::vector<VkDescriptorSet> &descriptorSets, u_GraphicsPipeline &graphicsPipeline, VkExtent2D &swapChainExtent, uint64_t instanceCount, uint32_t &currentFrame)
{
    
}

void GameObjectPool::cleanUpResources()
{
    
}

GameObjectPool::~GameObjectPool()
{
}
