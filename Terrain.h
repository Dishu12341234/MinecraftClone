#pragma once
#include "PassInfo.hpp"
#include "Structs.h"
#include "blocks.h"
#include "utils.h"
#include "GameObjectPool.h"
#include "Chunk.h"
#include <vector>

class Terrain
{
private:
    VulkanContext &vkContext;
    std::vector<Chunk> chunks;
    GameObjectPool &gameObjectPool;

    friend class GameObjectPool;

public:
    Terrain(const Terrain &) = delete;
    Terrain &operator=(const Terrain &other)
    {
        // allocate new Vulkan buffers, copy data
        return *this;
    }

    Terrain(Terrain &&) = default; // allow moves
    Terrain &operator=(Terrain &&) = default;
    Terrain(VulkanContext &vkContext, GameObjectPool &gameObjectPool);

    void generateChunks();
    void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline, std::vector<VkDescriptorSet> &descriptorSets, uint32_t currentFrame, VkExtent2D &swapChainExtent);

    void cleanup();
    ~Terrain();
};
